#pragma once

#include "common/types.h"

#include <QString>

namespace FabLink
{
struct GemEvent
{
    EventId id = 0;
    QString name;
    QString value;
};
}
