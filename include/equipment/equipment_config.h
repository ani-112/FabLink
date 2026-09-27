#pragma once

#include <QString>
#include <cstdint>

namespace FabLink
{
class EquipmentConfig
{
public:
    EquipmentConfig(QString equipmentId,
                    QString modelName,
                    QString softwareVersion,
                    std::uint16_t port);

    const QString& equipmentId() const;
    const QString& modelName() const;
    const QString& softwareVersion() const;
    std::uint16_t port() const;

private:
    QString m_equipmentId;
    QString m_modelName;
    QString m_softwareVersion;
    std::uint16_t m_port;
};
}
