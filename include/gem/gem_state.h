#pragma once

namespace FabLink
{
enum class GemState
{
    NotCommunicating,
    Communicating
};

enum class EquipmentControlState
{
    Offline,
    OnlineLocal,
    OnlineRemote
};
}
