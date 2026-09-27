#include "mainwindow.h"

#include "communication/communication_manager.h"
#include "gem/gem_event.h"
#include "gem/gem_state.h"

#include "ui_mainwindow.h"

#include <QDateTime>

namespace FabLink
{
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_communicationManager(new CommunicationManager(this))
{
    ui->setupUi(this);

    ui->equipmentIdLabel->setText(m_communicationManager->equipment().config().equipmentId());
    ui->modelLabel->setText(m_communicationManager->equipment().config().modelName());
    ui->portLabel->setText(QString::number(m_communicationManager->equipment().config().port()));

    connect(ui->startButton, &QPushButton::clicked,
            this, &MainWindow::startEquipment);
    connect(ui->stopButton, &QPushButton::clicked,
            this, &MainWindow::stopEquipment);

    connect(m_communicationManager, &CommunicationManager::logMessage,
            this, &MainWindow::appendLog);
    connect(m_communicationManager, &CommunicationManager::connectionStatusChanged,
            this, &MainWindow::updateConnectionStatus);
    connect(m_communicationManager, &CommunicationManager::eventGenerated,
            this, &MainWindow::showEvent);

    connect(&m_communicationManager->gemStateMachine(),
            &GemStateMachine::stateChanged,
            this,
            &MainWindow::updateGemState);

    updateConnectionStatus(false);
    appendLog(QStringLiteral("FabLink initialized."));
}

MainWindow::~MainWindow()
{
    m_communicationManager->stopEquipment();
    delete ui;
}

void MainWindow::startEquipment()
{
    m_communicationManager->startEquipment();
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
}

void MainWindow::stopEquipment()
{
    m_communicationManager->stopEquipment();
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
}

void MainWindow::appendLog(const QString& message)
{
    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss"));
    ui->logTextEdit->appendPlainText(
        QStringLiteral("[%1] %2").arg(timestamp, message));
}

void MainWindow::updateConnectionStatus(bool connected)
{
    ui->connectionStatusLabel->setText(
        connected ? QStringLiteral("CONNECTED")
                  : QStringLiteral("DISCONNECTED"));
}

void MainWindow::showEvent(const FabLink::GemEvent& event)
{
    ui->lastEventLabel->setText(
        QStringLiteral("%1 = %2").arg(event.name, event.value));
}

void MainWindow::updateGemState(
    FabLink::GemState communication,
    FabLink::EquipmentControlState control)
{
    const QString communicationText =
        communication == GemState::Communicating
            ? QStringLiteral("COMMUNICATING")
            : QStringLiteral("NOT_COMMUNICATING");

    const QString controlText =
        control == EquipmentControlState::OnlineRemote
            ? QStringLiteral("ONLINE_REMOTE")
            : control == EquipmentControlState::OnlineLocal
                ? QStringLiteral("ONLINE_LOCAL")
                : QStringLiteral("OFFLINE");

    ui->gemStateLabel->setText(
        QStringLiteral("%1 / %2").arg(communicationText, controlText));
}
}
