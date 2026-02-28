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

FrameCapture::FrameCapture(const QString &preset, QColor displayColor, QObject *parent)
    : ModuleBase(parent)
    , CaptureHolder(preset, displayColor)
{}

void FrameCapture::stop()
{
    QMutexLocker workLocker(&m_workMutex);
    ModuleBase::stop();
    m_condition.wakeOne();
}

void FrameCapture::run()
{
    if (m_mode == Mode::Invalid)
    {
        m_error = "Preset \"" + m_preset + "\" does not exist";
        m_result = -1;
        return;
    }

    if (!m_preset.isEmpty())
    {
        PrintLog("Detecting with preset \"" + m_preset + "\"");
    }

    QColor pixel;
    QImage frame;
    while (m_result == 0 && !m_terminate)
    {
        {
            // wait for work
            QMutexLocker workLocker(&m_workMutex);
            m_pendingWork = false;
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
            std::unique_lock lock(m_resultMutex);
            switch (m_mode)
            {
            case CaptureHolder::Mode::PointColorMatch:
            {
                QColor const target = GetTargetColor();
                m_resultColor = pixel;
                m_resultMatched = GetColorMatch(pixel, target);
                m_resultString = m_resultColor.name().toUpper();

                lock.unlock();
                if (m_terminate) return;
                emit notifyResultMatched(m_resultMatched);
                break;
            }
            case CaptureHolder::Mode::PointRangeMatch:
            {
                HsvRange const range = GetHsvRange();
                m_resultColor = pixel.toHsv();
                m_resultMatched = GetColorMatchHSV(pixel, range);
                m_resultString = m_resultColor.name().toUpper();

                lock.unlock();
                if (m_terminate) return;
                emit notifyResultMatched(m_resultMatched);
                break;
            }
            case CaptureHolder::Mode::AreaColorMatch:
            {
                QColor const target = GetTargetColor();
                m_resultColor = GetAverageColor(frame);
                m_resultMatched = GetColorMatch(m_resultColor, target);
                m_resultString = m_resultColor.name().toUpper();

                lock.unlock();
                if (m_terminate) return;
                emit notifyResultMatched(m_resultMatched);
                break;
            }
            case CaptureHolder::Mode::AreaRangeMatch:
            {
                HsvRange const range = GetHsvRange();
                m_resultMean = GetBrightnessMean(frame, range, &m_resultMasked);
                m_resultString = QString::number(m_resultMean, 'f', 4);
                m_resultMatched = m_resultMean >= m_targetMean;

                lock.unlock();
                if (m_terminate) return;
                emit notifyResultMatched(m_resultMatched);
                emit notifyResultMean(m_resultMean, m_resultMasked);
                break;
            }
            default: break;
            }
        }
    }
}

}
