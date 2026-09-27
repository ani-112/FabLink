#pragma once

#include "gem/gem_state.h"

#include <QObject>

namespace FabLink
{
class GemStateMachine : public QObject
{
    Q_OBJECT

public:
    explicit GemStateMachine(QObject* parent = nullptr);

    GemState communicationState() const;
    EquipmentControlState controlState() const;

public slots:
    void setCommunicating();
    void setNotCommunicating();
    void setControlState(EquipmentControlState state);

signals:
    void stateChanged(FabLink::GemState communicationState,
                      FabLink::EquipmentControlState controlState);

private:
    GemState m_communicationState = GemState::NotCommunicating;
    EquipmentControlState m_controlState = EquipmentControlState::Offline;
};
}
