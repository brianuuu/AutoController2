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

void BrightnessMean::PushFrameData(const QImage &frame)
{
    QMutexLocker locker(&m_workMutex);
    if (m_pendingWork) return;

    CaptureHolder::PushFrameData(frame);
    m_pendingWork = true;
}

void BrightnessMean::run()
{
    QImage frame;
    while (!m_terminate)
    {
        {
            // wait for work
            QMutexLocker workLocker(&m_workMutex);
            if (!m_pendingWork)
            {
                // TODO: use condition variable
                continue;
            }
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

qreal BrightnessMean::GetResultMean() const
{
    QMutexLocker locker(&m_resultMutex);
    return m_resultMean;
}

}
