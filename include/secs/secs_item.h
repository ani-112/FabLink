#pragma once

#include <QByteArray>
#include <QString>
#include <QVector>
#include <cstdint>
#include <memory>
#include <variant>

namespace FabLink
{
enum class SecsFormat : std::uint8_t
{
    List,
    Binary,
    Boolean,
    U1,
    U2,
    U4,
    U8,
    ASCII
};

class SecsItem
{
public:
    using List = QVector<SecsItem>;
    using Value = std::variant<List, QByteArray, bool, std::uint8_t,
                               std::uint16_t, std::uint32_t, std::uint64_t, QString>;

    static SecsItem list(List items = {});
    static SecsItem binary(const QByteArray& value);
    static SecsItem boolean(bool value);
    static SecsItem u1(std::uint8_t value);
    static SecsItem u2(std::uint16_t value);
    static SecsItem u4(std::uint32_t value);
    static SecsItem u8(std::uint64_t value);
    static SecsItem ascii(const QString& value);

    SecsFormat format() const;
    const Value& value() const;

    QByteArray encode() const;
    static bool decode(const QByteArray& bytes, SecsItem& item);

private:
    SecsItem(SecsFormat format, Value value);

    static QByteArray encodeLength(std::uint32_t length, std::uint8_t formatCode);
    static bool decodeItem(const QByteArray& bytes, int& offset, SecsItem& item);

    SecsFormat m_format;
    Value m_value;
};
}
