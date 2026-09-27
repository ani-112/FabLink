
#include "equipment/measurement_worker.h"

#include <QRandomGenerator>

namespace FabLink
{
MeasurementWorker::MeasurementWorker(QObject* parent)
    : QObject(parent)
{
}

void MeasurementWorker::start()
{
    if (!m_timer)
    {
        m_timer = new QTimer(this);
        m_timer->setInterval(2000);

        connect(m_timer, &QTimer::timeout, this, [this]() {
            const int value = 90 + QRandomGenerator::global()->bounded(20);
            emit measurementReady(value);
        });
    }

    m_timer->start();
}

void MeasurementWorker::stop()
{
    if (m_timer)
        m_timer->stop();
}
}
