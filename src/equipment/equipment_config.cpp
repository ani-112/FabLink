#include "equipment/equipment_config.h"
#include <utility>

namespace FabLink
{
EquipmentConfig::EquipmentConfig(QString equipmentId,
                                 QString modelName,
                                 QString softwareVersion,
                                 std::uint16_t port)
    : m_equipmentId(std::move(equipmentId))
    , m_modelName(std::move(modelName))
    , m_softwareVersion(std::move(softwareVersion))
    , m_port(port)
{
}

const QString& EquipmentConfig::equipmentId() const { return m_equipmentId; }
const QString& EquipmentConfig::modelName() const { return m_modelName; }
const QString& EquipmentConfig::softwareVersion() const { return m_softwareVersion; }
std::uint16_t EquipmentConfig::port() const { return m_port; }
}
