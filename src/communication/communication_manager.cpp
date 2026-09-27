#include "communication/communication_manager.h"
#include "equipment/measurement_worker.h"

#include <QRandomGenerator>
#include <QString>

namespace FabLink
{
CommunicationManager::CommunicationManager(QObject* parent)
    : QObject(parent)
    , m_equipment(EquipmentConfig(
          QStringLiteral("FSM-EQ-001"),
          QStringLiteral("FSM Metrology Tool"),
          QStringLiteral("1.0.0"),
          5000))
{
    connect(&m_hsms, &HsmsConnection::messageReceived,
            this, &CommunicationManager::onHsmsMessage);
    connect(&m_hsms, &HsmsConnection::connected,
            this, &CommunicationManager::onHsmsConnected);
    connect(&m_hsms, &HsmsConnection::disconnected,
            this, &CommunicationManager::onHsmsDisconnected);
    connect(&m_hsms, &HsmsConnection::logMessage,
            this, &CommunicationManager::logMessage);

    connect(&m_gem, &GemStateMachine::stateChanged,
            this, [this](GemState communication, EquipmentControlState control) {
                const QString comm = communication == GemState::Communicating
                    ? QStringLiteral("COMMUNICATING")
                    : QStringLiteral("NOT_COMMUNICATING");
                const QString ctrl = control == EquipmentControlState::OnlineRemote
                    ? QStringLiteral("ONLINE_REMOTE")
                    : control == EquipmentControlState::OnlineLocal
                        ? QStringLiteral("ONLINE_LOCAL")
                        : QStringLiteral("OFFLINE");
                emit logMessage(QStringLiteral("GEM state: %1 / %2").arg(comm, ctrl));
            });

    m_measurementWorker = new MeasurementWorker();
    m_measurementWorker->moveToThread(&m_measurementThread);

    connect(&m_measurementThread, &QThread::finished,
            m_measurementWorker, &QObject::deleteLater);
    connect(m_measurementWorker, &MeasurementWorker::measurementReady,
            this, &CommunicationManager::onMeasurementReady,
            Qt::QueuedConnection);

    m_measurementThread.start();
}


CommunicationManager::~CommunicationManager()
{
    if (m_measurementWorker)
        QMetaObject::invokeMethod(m_measurementWorker, "stop", Qt::QueuedConnection);
    m_measurementThread.quit();
    m_measurementThread.wait();
}

void CommunicationManager::startEquipment()
{
    m_equipment.start();

    if (m_hsms.listen(m_equipment.config().port()))
    {
        emit logMessage(QStringLiteral("Equipment %1 started")
                            .arg(m_equipment.config().equipmentId()));
    }
}

void CommunicationManager::stopEquipment()
{
    if (m_measurementWorker)
        QMetaObject::invokeMethod(m_measurementWorker, "stop", Qt::QueuedConnection);
    m_hsms.close();
    m_gem.setNotCommunicating();
    m_equipment.stop();
    emit logMessage(QStringLiteral("Equipment stopped"));
}

const Equipment& CommunicationManager::equipment() const { return m_equipment; }
GemStateMachine& CommunicationManager::gemStateMachine() { return m_gem; }
const GemStateMachine& CommunicationManager::gemStateMachine() const { return m_gem; }

void CommunicationManager::onHsmsConnected()
{
    emit connectionStatusChanged(true);
}

void CommunicationManager::onHsmsDisconnected()
{
    if (m_measurementWorker)
        QMetaObject::invokeMethod(m_measurementWorker, "stop", Qt::QueuedConnection);
    m_gem.setNotCommunicating();
    emit connectionStatusChanged(false);
}

void CommunicationManager::onHsmsMessage(const HsmsMessage& message)
{
    if (!message.isDataMessage())
    {
        if (message.sessionType() == 1)
        {
            m_hsms.send(HsmsMessage::selectResponse(message.system()));
            emit logMessage(QStringLiteral("HSMS Select.req -> Select.rsp"));
        }
        else if (message.sessionType() == 5)
        {
            m_hsms.send(HsmsMessage::linktestResponse(message.system()));
            emit logMessage(QStringLiteral("HSMS Linktest.req -> Linktest.rsp"));
        }
        return;
    }

    const Stream stream = static_cast<Stream>(message.streamByte() & 0x7F);
    const Function function = message.function();

    emit logMessage(QStringLiteral("Received S%1F%2").arg(stream).arg(function));

    if (stream == 1 && function == 1)
    {
        sendS1F2(message.system());
    }
    else if (stream == 1 && function == 13)
    {
        sendS1F14(message.system());
        m_gem.setCommunicating();
        m_gem.setControlState(EquipmentControlState::OnlineRemote);
        if (m_measurementWorker)
            QMetaObject::invokeMethod(m_measurementWorker, "start", Qt::QueuedConnection);
    }
    else if (stream == 6 && function == 12)
    {
        emit logMessage(QStringLiteral("Event report acknowledged (S6F12)"));
    }
}

void CommunicationManager::sendS1F2(std::uint32_t system)
{
    SecsItem body = SecsItem::list({
        SecsItem::ascii(m_equipment.config().modelName()),
        SecsItem::ascii(m_equipment.config().softwareVersion())
    });

    SecsMessage message(1, 2, false, system, body);
    m_hsms.send(HsmsMessage::data(message));
    emit logMessage(QStringLiteral("Sent S1F2"));
}

void CommunicationManager::sendS1F14(std::uint32_t system)
{
    SecsItem body = SecsItem::list({
        SecsItem::u1(0) // COMMACK = accepted
    });

    SecsMessage message(1, 14, false, system, body);
    m_hsms.send(HsmsMessage::data(message));
    emit logMessage(QStringLiteral("Sent S1F14 (COMMACK=0)"));
}

void CommunicationManager::onMeasurementReady(int value)
{
    if (m_gem.communicationState() != GemState::Communicating)
        return;

    const EventId eventId = m_nextEventId++;
    SecsItem report = SecsItem::list({
        SecsItem::u4(eventId),
        SecsItem::ascii(QStringLiteral("WaferMeasurement")),
        SecsItem::u4(static_cast<std::uint32_t>(value))
    });

    SecsMessage message(6, 11, true, m_nextSystem++, report);
    m_hsms.send(HsmsMessage::data(message));

    GemEvent event;
    event.id = eventId;
    event.name = QStringLiteral("WaferMeasurement");
    event.value = QString::number(value);
    emit eventGenerated(event);
    emit logMessage(QStringLiteral("Sent S6F11: measurement=%1").arg(value));
}
}
