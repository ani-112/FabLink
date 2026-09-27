#include "hsms/hsms_message.h"

namespace FabLink
{
namespace
{
void appendU16(QByteArray& out, std::uint16_t value)
{
    out.append(char((value >> 8) & 0xFF));
    out.append(char(value & 0xFF));
}

void appendU32(QByteArray& out, std::uint32_t value)
{
    out.append(char((value >> 24) & 0xFF));
    out.append(char((value >> 16) & 0xFF));
    out.append(char((value >> 8) & 0xFF));
    out.append(char(value & 0xFF));
}

std::uint16_t readU16(const QByteArray& data, int offset)
{
    return (static_cast<std::uint16_t>(static_cast<std::uint8_t>(data[offset])) << 8)
           | static_cast<std::uint8_t>(data[offset + 1]);
}

std::uint32_t readU32(const QByteArray& data, int offset)
{
    return (static_cast<std::uint32_t>(static_cast<std::uint8_t>(data[offset])) << 24)
           | (static_cast<std::uint32_t>(static_cast<std::uint8_t>(data[offset + 1])) << 16)
           | (static_cast<std::uint32_t>(static_cast<std::uint8_t>(data[offset + 2])) << 8)
           | static_cast<std::uint8_t>(data[offset + 3]);
}
}

HsmsMessage::HsmsMessage() = default;

HsmsMessage HsmsMessage::data(const SecsMessage& message)
{
    HsmsMessage result;
    result.m_sessionId = 0;
    result.m_streamByte = static_cast<std::uint8_t>(message.stream() & 0x7F);
    if (message.waitBit())
        result.m_streamByte |= 0x80;
    result.m_function = message.function();
    result.m_pType = 0;
    result.m_sessionType = 0;
    result.m_system = message.system();
    result.m_body = message.encodeBody();
    return result;
}

HsmsMessage HsmsMessage::selectRequest(std::uint32_t system)
{
    HsmsMessage result;
    result.m_sessionId = 0xFFFF;
    result.m_sessionType = 1;
    result.m_system = system;
    return result;
}

HsmsMessage HsmsMessage::selectResponse(std::uint32_t system)
{
    HsmsMessage result;
    result.m_sessionId = 0xFFFF;
    result.m_sessionType = 2;
    result.m_system = system;
    result.m_body = QByteArray(1, char(0)); // accepted
    return result;
}

HsmsMessage HsmsMessage::linktestRequest(std::uint32_t system)
{
    HsmsMessage result;
    result.m_sessionId = 0xFFFF;
    result.m_sessionType = 5;
    result.m_system = system;
    return result;
}

HsmsMessage HsmsMessage::linktestResponse(std::uint32_t system)
{
    HsmsMessage result;
    result.m_sessionId = 0xFFFF;
    result.m_sessionType = 6;
    result.m_system = system;
    return result;
}

bool HsmsMessage::isDataMessage() const { return m_sessionType == 0; }
std::uint16_t HsmsMessage::sessionId() const { return m_sessionId; }
std::uint8_t HsmsMessage::streamByte() const { return m_streamByte; }
std::uint8_t HsmsMessage::function() const { return m_function; }
std::uint8_t HsmsMessage::pType() const { return m_pType; }
std::uint8_t HsmsMessage::sessionType() const { return m_sessionType; }
std::uint32_t HsmsMessage::system() const { return m_system; }
const QByteArray& HsmsMessage::body() const { return m_body; }

QByteArray HsmsMessage::encode() const
{
    QByteArray header;
    appendU16(header, m_sessionId);
    header.append(char(m_streamByte));
    header.append(char(m_function));
    header.append(char(m_pType));
    header.append(char(m_sessionType));
    appendU32(header, m_system);

    QByteArray frame;
    appendU32(frame, static_cast<std::uint32_t>(header.size() + m_body.size()));
    frame.append(header);
    frame.append(m_body);
    return frame;
}

bool HsmsMessage::decode(const QByteArray& frame, HsmsMessage& message)
{
    if (frame.size() < 14)
        return false;

    const std::uint32_t length = readU32(frame, 0);
    if (length != static_cast<std::uint32_t>(frame.size() - 4) || length < 10)
        return false;

    message.m_sessionId = readU16(frame, 4);
    message.m_streamByte = static_cast<std::uint8_t>(frame.at(6));
    message.m_function = static_cast<std::uint8_t>(frame.at(7));
    message.m_pType = static_cast<std::uint8_t>(frame.at(8));
    message.m_sessionType = static_cast<std::uint8_t>(frame.at(9));
    message.m_system = readU32(frame, 10);
    message.m_body = frame.mid(14);
    return true;
}
}
