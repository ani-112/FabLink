
#pragma once

#include <QObject>
#include <QTimer>

namespace FabLink
{
class MeasurementWorker : public QObject
{
    Q_OBJECT

public:
    explicit MeasurementWorker(QObject* parent = nullptr);

public slots:
    void start();
    void stop();

signals:
    void measurementReady(int value);

private:
    QTimer* m_timer = nullptr;
};
}
