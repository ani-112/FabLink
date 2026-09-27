#pragma once

#include "secs/secs_message.h"

#include <QByteArray>
#include <cstdint>

namespace FabLink
{
enum class HsmsSessionType : std::uint8_t
{
    DataMessage = 0,
    SelectRequest = 1,
    SelectResponse = 2,
    DeselectRequest = 3,
    DeselectResponse = 4,
    LinktestRequest = 5,
    LinktestResponse = 6
};

class HsmsMessage
{
public:
    HsmsMessage();

    static HsmsMessage data(const SecsMessage& message);
    static HsmsMessage selectRequest(std::uint32_t system);
    static HsmsMessage selectResponse(std::uint32_t system);
    static HsmsMessage linktestRequest(std::uint32_t system);
    static HsmsMessage linktestResponse(std::uint32_t system);

    bool isDataMessage() const;
    std::uint16_t sessionId() const;
    std::uint8_t streamByte() const;
    std::uint8_t function() const;
    std::uint8_t pType() const;
    std::uint8_t sessionType() const;
    std::uint32_t system() const;
    const QByteArray& body() const;

    QByteArray encode() const;
    static bool decode(const QByteArray& frame, HsmsMessage& message);

private:
    std::uint16_t m_sessionId = 0;
    std::uint8_t m_streamByte = 0;
    std::uint8_t m_function = 0;
    std::uint8_t m_pType = 0;
    std::uint8_t m_sessionType = 0;
    std::uint32_t m_system = 0;
    QByteArray m_body;
};
}
