#pragma once

#include "equipment/equipment.h"
#include "gem/gem_event.h"
#include "gem/gem_state_machine.h"
#include "hsms/hsms_connection.h"
#include "equipment/measurement_worker.h"

#include <QObject>
#include <QThread>

namespace FabLink
{
class CommunicationManager : public QObject
{
    Q_OBJECT

public:
    explicit CommunicationManager(QObject* parent = nullptr);
    ~CommunicationManager() override;

    void startEquipment();
    void stopEquipment();

    const Equipment& equipment() const;
    GemStateMachine& gemStateMachine();
    const GemStateMachine& gemStateMachine() const;

signals:
    void logMessage(const QString& message);
    void eventGenerated(const FabLink::GemEvent& event);
    void connectionStatusChanged(bool connected);

private slots:
    void onHsmsMessage(const FabLink::HsmsMessage& message);
    void onHsmsConnected();
    void onHsmsDisconnected();
    void onMeasurementReady(int value);

private:
    void sendS1F2(std::uint32_t system);
    void sendS1F14(std::uint32_t system);

    Equipment m_equipment;
    HsmsConnection m_hsms;
    GemStateMachine m_gem;
    QThread m_measurementThread;
    EventId m_nextEventId = 1;
    std::uint32_t m_nextSystem = 1;
    MeasurementWorker* m_measurementWorker = nullptr;
};
}
