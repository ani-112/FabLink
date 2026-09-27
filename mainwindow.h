#pragma once

#include <QMainWindow>

#include "gem/gem_event.h"
#include "gem/gem_state.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

namespace FabLink
{
class CommunicationManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void startEquipment();
    void stopEquipment();
    void appendLog(const QString& message);
    void updateConnectionStatus(bool connected);
    void showEvent(const FabLink::GemEvent& event);
    void updateGemState(FabLink::GemState communication,
                        FabLink::EquipmentControlState control);

private:
    Ui::MainWindow* ui;
    CommunicationManager* m_communicationManager;
};
}
