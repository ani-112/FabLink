#pragma once

#include "common/types.h"
#include "secs/secs_item.h"

namespace FabLink
{
class SecsMessage
{
public:
    SecsMessage(Stream stream,
                Function function,
                bool waitBit,
                TransactionId system,
                SecsItem body = SecsItem::list());

    Stream stream() const;
    Function function() const;
    bool waitBit() const;
    TransactionId system() const;
    const SecsItem& body() const;

    QByteArray encodeBody() const;

private:
    Stream m_stream;
    Function m_function;
    bool m_waitBit;
    TransactionId m_system;
    SecsItem m_body;
};
}
