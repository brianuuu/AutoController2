#include "framecapture.h"

namespace Module::Common
{

FrameCapture::FrameCapture(QPoint point, QColor testColor, QColor displayColor, QObject *parent)
    : ModuleBase(parent)
    , CaptureHolder(point, testColor, displayColor)
{}

FrameCapture::FrameCapture(QPoint point, HsvRange range, QColor displayColor, QObject *parent)
    : ModuleBase(parent)
    , CaptureHolder(point, range, displayColor)
{}

FrameCapture::FrameCapture(QRect rect, QColor testColor, QColor displayColor, QObject *parent)
    : ModuleBase(parent)
    , CaptureHolder(rect, testColor, displayColor)
{}

FrameCapture::FrameCapture(QRect rect, HsvRange range, QColor displayColor, QObject *parent)
    : ModuleBase(parent)
    , CaptureHolder(rect, range, displayColor)
{}

void FrameCapture::stop()
{
    QMutexLocker workLocker(&m_workMutex);
    ModuleBase::stop();
    m_condition.wakeOne();
}

void FrameCapture::PushFrameData(const QImage &frame)
{
    QMutexLocker locker(&m_workMutex);
    if (m_pendingWork) return;

    CaptureHolder::PushFrameData(frame);

    m_pendingWork = true;
    m_condition.wakeOne();
}

void FrameCapture::run()
{
    QColor pixel;
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
            pixel = GetPixelData();
            frame = GetFrameData();
        }
        {
            // analyze
            QMutexLocker resultLocker(&m_resultMutex);
            switch (m_mode)
            {
            case CaptureHolder::Mode::PointColorMatch:
            {
                QColor const target = GetTargetColor();
                m_resultColor = pixel;
                m_resultMatched = GetColorMatch(pixel, target);
                break;
            }
            case CaptureHolder::Mode::PointRangeMatch:
            {
                HsvRange const range = GetHsvRange();
                m_resultColor = pixel.toHsv();
                m_resultMatched = GetColorMatchHSV(pixel, range);
                break;
            }
            case CaptureHolder::Mode::AreaColorMatch:
            {
                QColor const target = GetTargetColor();
                m_resultColor = GetAverageColor(frame);
                m_resultMatched = GetColorMatch(m_resultColor, target);
                break;
            }
            case CaptureHolder::Mode::AreaRangeMatch:
            {
                HsvRange const range = GetHsvRange();
                m_resultMean = GetBrightnessMean(frame, range, &m_resultMasked);
                break;
            }
            }

            // this is free to do next work
            QMutexLocker workLocker(&m_workMutex);
            m_pendingWork = false;
        }
    }
}

}
