#include "secs/secs_item.h"

#include <QDataStream>
#include <QIODevice>

namespace FabLink
{
namespace
{
std::uint8_t formatCode(SecsFormat format)
{
    switch (format)
    {
    case SecsFormat::List: return 0x01;
    case SecsFormat::Binary: return 0x21;
    case SecsFormat::Boolean: return 0x25;
    case SecsFormat::U1: return 0xA5;
    case SecsFormat::U2: return 0xA9;
    case SecsFormat::U4: return 0x91;
    case SecsFormat::U8: return 0x81;
    case SecsFormat::ASCII: return 0x41;
    }
    return 0;
}

int elementSize(SecsFormat format)
{
    switch (format)
    {
    case SecsFormat::Binary:
    case SecsFormat::Boolean:
    case SecsFormat::U1:
    case SecsFormat::ASCII: return 1;
    case SecsFormat::U2: return 2;
    case SecsFormat::U4: return 4;
    case SecsFormat::U8: return 8;
    case SecsFormat::List: return 0;
    }
    return 0;
}
}

SecsItem::SecsItem(SecsFormat format, Value value)
    : m_format(format), m_value(std::move(value))
{
}

SecsItem SecsItem::list(List items) { return {SecsFormat::List, std::move(items)}; }
SecsItem SecsItem::binary(const QByteArray& value) { return {SecsFormat::Binary, value}; }
SecsItem SecsItem::boolean(bool value) { return {SecsFormat::Boolean, value}; }
SecsItem SecsItem::u1(std::uint8_t value) { return {SecsFormat::U1, value}; }
SecsItem SecsItem::u2(std::uint16_t value) { return {SecsFormat::U2, value}; }
SecsItem SecsItem::u4(std::uint32_t value) { return {SecsFormat::U4, value}; }
SecsItem SecsItem::u8(std::uint64_t value) { return {SecsFormat::U8, value}; }
SecsItem SecsItem::ascii(const QString& value) { return {SecsFormat::ASCII, value}; }

SecsFormat SecsItem::format() const { return m_format; }
const SecsItem::Value& SecsItem::value() const { return m_value; }

QByteArray SecsItem::encodeLength(std::uint32_t length, std::uint8_t formatCodeValue)
{
    QByteArray out;
    if (length <= 0xFF)
    {
        out.append(static_cast<char>(formatCodeValue | 0x01));
        out.append(static_cast<char>(length));
    }
    else if (length <= 0xFFFF)
    {
        out.append(static_cast<char>(formatCodeValue | 0x02));
        out.append(static_cast<char>((length >> 8) & 0xFF));
        out.append(static_cast<char>(length & 0xFF));
    }
    else
    {
        out.append(static_cast<char>(formatCodeValue | 0x03));
        out.append(static_cast<char>((length >> 16) & 0xFF));
        out.append(static_cast<char>((length >> 8) & 0xFF));
        out.append(static_cast<char>(length & 0xFF));
    }
    return out;
}

QByteArray SecsItem::encode() const
{
    QByteArray payload;
    std::uint32_t itemCount = 1;

    switch (m_format)
    {
    case SecsFormat::List:
    {
        const auto& items = std::get<List>(m_value);
        payload.append(char(0)); // overwritten below only for clarity
        payload.clear();
        for (const auto& item : items)
            payload.append(item.encode());
        itemCount = static_cast<std::uint32_t>(items.size());
        break;
    }
    case SecsFormat::Binary:
        payload = std::get<QByteArray>(m_value);
        itemCount = static_cast<std::uint32_t>(payload.size());
        break;
    case SecsFormat::Boolean:
        payload.append(std::get<bool>(m_value) ? char(1) : char(0));
        break;
    case SecsFormat::U1:
        payload.append(char(std::get<std::uint8_t>(m_value)));
        break;
    case SecsFormat::U2:
    {
        const auto value = std::get<std::uint16_t>(m_value);
        payload.append(char((value >> 8) & 0xFF));
        payload.append(char(value & 0xFF));
        break;
    }
    case SecsFormat::U4:
    {
        const auto value = std::get<std::uint32_t>(m_value);
        for (int shift = 24; shift >= 0; shift -= 8)
            payload.append(char((value >> shift) & 0xFF));
        break;
    }
    case SecsFormat::U8:
    {
        const auto value = std::get<std::uint64_t>(m_value);
        for (int shift = 56; shift >= 0; shift -= 8)
            payload.append(char((value >> shift) & 0xFF));
        break;
    }
    case SecsFormat::ASCII:
        payload = std::get<QString>(m_value).toUtf8();
        itemCount = static_cast<std::uint32_t>(payload.size());
        break;
    }

    const std::uint8_t code = formatCode(m_format);
    const std::uint32_t length = (m_format == SecsFormat::List)
        ? itemCount
        : static_cast<std::uint32_t>(payload.size());

    QByteArray result = encodeLength(length, code);
    result.append(payload);
    return result;
}

bool SecsItem::decodeItem(const QByteArray& bytes, int& offset, SecsItem& item)
{
    if (offset >= bytes.size())
        return false;

    const std::uint8_t first = static_cast<std::uint8_t>(bytes.at(offset++));
    const std::uint8_t format = first & 0xFC;
    const int lengthBytes = first & 0x03;

    if (lengthBytes < 1 || lengthBytes > 3 || offset + lengthBytes > bytes.size())
        return false;

    std::uint32_t length = 0;
    for (int i = 0; i < lengthBytes; ++i)
        length = (length << 8) | static_cast<std::uint8_t>(bytes.at(offset++));

    const int payloadStart = offset;

    SecsFormat secFormat;
    switch (format)
    {
    case 0x00: secFormat = SecsFormat::List; break;
    case 0x20: secFormat = SecsFormat::Binary; break;
    case 0x24: secFormat = SecsFormat::Boolean; break;
    case 0xA4: secFormat = SecsFormat::U1; break;
    case 0xA8: secFormat = SecsFormat::U2; break;
    case 0x90: secFormat = SecsFormat::U4; break;
    case 0x80: secFormat = SecsFormat::U8; break;
    case 0x40: secFormat = SecsFormat::ASCII; break;
    default: return false;
    }

    if (secFormat == SecsFormat::List)
    {
        List items;
        for (std::uint32_t i = 0; i < length; ++i)
        {
            SecsItem child = SecsItem::list();
            if (!decodeItem(bytes, offset, child))
                return false;
            items.append(child);
        }
        item = SecsItem::list(std::move(items));
        return true;
    }

    const int size = elementSize(secFormat);
    const qint64 payloadBytes = (secFormat == SecsFormat::ASCII || secFormat == SecsFormat::Binary)
        ? length
        : static_cast<qint64>(length) * size;

    if (payloadBytes < 0 || offset + payloadBytes > bytes.size())
        return false;

    const QByteArray payload = bytes.mid(offset, static_cast<int>(payloadBytes));
    offset += static_cast<int>(payloadBytes);

    switch (secFormat)
    {
    case SecsFormat::Binary:
        item = binary(payload);
        break;
    case SecsFormat::ASCII:
        item = ascii(QString::fromUtf8(payload));
        break;
    case SecsFormat::Boolean:
        item = boolean(!payload.isEmpty() && payload.at(0) != 0);
        break;
    case SecsFormat::U1:
        item = u1(static_cast<std::uint8_t>(payload.at(0)));
        break;
    case SecsFormat::U2:
        item = u2((static_cast<std::uint16_t>(static_cast<std::uint8_t>(payload.at(0))) << 8)
                  | static_cast<std::uint8_t>(payload.at(1)));
        break;
    case SecsFormat::U4:
    {
        std::uint32_t value = 0;
        for (char c : payload)
            value = (value << 8) | static_cast<std::uint8_t>(c);
        item = u4(value);
        break;
    }
    case SecsFormat::U8:
    {
        std::uint64_t value = 0;
        for (char c : payload)
            value = (value << 8) | static_cast<std::uint8_t>(c);
        item = u8(value);
        break;
    }
    case SecsFormat::List:
        break;
    }

    Q_UNUSED(payloadStart);
    return true;
}

bool SecsItem::decode(const QByteArray& bytes, SecsItem& item)
{
    int offset = 0;
    return decodeItem(bytes, offset, item) && offset == bytes.size();
}
}
