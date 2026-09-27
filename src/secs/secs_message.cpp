#include "secs/secs_message.h"

namespace FabLink
{
SecsMessage::SecsMessage(Stream stream,
                         Function function,
                         bool waitBit,
                         TransactionId system,
                         SecsItem body)
    : m_stream(stream)
    , m_function(function)
    , m_waitBit(waitBit)
    , m_system(system)
    , m_body(std::move(body))
{
}

Stream SecsMessage::stream() const { return m_stream; }
Function SecsMessage::function() const { return m_function; }
bool SecsMessage::waitBit() const { return m_waitBit; }
TransactionId SecsMessage::system() const { return m_system; }
const SecsItem& SecsMessage::body() const { return m_body; }
QByteArray SecsMessage::encodeBody() const { return m_body.encode(); }
}
