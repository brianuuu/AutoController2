#include "brightnessmean.h"
#include <QDebug>

namespace Module::Common
{

BrightnessMean::BrightnessMean
(
    QRect rect,
    HsvRange range,
    QColor color,
    QObject *parent
)
    : ModuleBase(parent)
    , CaptureHolder(rect, range, color)
{}

void BrightnessMean::stop()
{
    QMutexLocker workLocker(&m_workMutex);
    ModuleBase::stop();
    m_condition.wakeOne();
}

void BrightnessMean::PushFrameData(const QImage &frame)
{
    QMutexLocker locker(&m_workMutex);
    if (m_pendingWork) return;

    CaptureHolder::PushFrameData(frame);

    m_pendingWork = true;
    m_condition.wakeOne();
}

void BrightnessMean::run()
{
    QImage frame;
    while (!m_terminate)
    {
        {
            // wait for work
            QMutexLocker workLocker(&m_workMutex);
            while (!m_pendingWork && !m_terminate)
            {
                m_condition.wait(&m_workMutex);
            }

            if (m_terminate) return;
            frame = GetFrameData();
        }
        {
            // analyze
            QMutexLocker resultLocker(&m_resultMutex);
            HsvRange const range = GetHsvRange();
            m_resultMean = GetBrightnessMean(frame, range, &m_resultMasked);

            // this is free to do next work
            QMutexLocker workLocker(&m_workMutex);
            m_pendingWork = false;
        }
    }
}

}
