#include "equipment/equipment.h"
#include <utility>

namespace FabLink
{
Equipment::Equipment(EquipmentConfig config)
    : m_config(std::move(config))
{
}

void Equipment::start() { m_running = true; }
void Equipment::stop() { m_running = false; }
bool Equipment::isRunning() const { return m_running; }
const EquipmentConfig& Equipment::config() const { return m_config; }
}
