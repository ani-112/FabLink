#include "gem/gem_state_machine.h"

namespace FabLink
{
GemStateMachine::GemStateMachine(QObject* parent)
    : QObject(parent)
{
}

GemState GemStateMachine::communicationState() const { return m_communicationState; }
EquipmentControlState GemStateMachine::controlState() const { return m_controlState; }

void GemStateMachine::setCommunicating()
{
    if (m_communicationState == GemState::Communicating)
        return;
    m_communicationState = GemState::Communicating;
    emit stateChanged(m_communicationState, m_controlState);
}

void GemStateMachine::setNotCommunicating()
{
    if (m_communicationState == GemState::NotCommunicating)
        return;
    m_communicationState = GemState::NotCommunicating;
    m_controlState = EquipmentControlState::Offline;
    emit stateChanged(m_communicationState, m_controlState);
}

void GemStateMachine::setControlState(EquipmentControlState state)
{
    if (m_controlState == state)
        return;
    m_controlState = state;
    emit stateChanged(m_communicationState, m_controlState);
}
}
