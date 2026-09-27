#pragma once

#include "equipment/equipment_config.h"

namespace FabLink
{
class Equipment
{
public:
    explicit Equipment(EquipmentConfig config);

    void start();
    void stop();
    bool isRunning() const;

    const EquipmentConfig& config() const;

private:
    EquipmentConfig m_config;
    bool m_running = false;
};
}
