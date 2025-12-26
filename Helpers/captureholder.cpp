#include "captureholder.h"

#include "Managers/managercollection.h"
#include "Managers/videomanager.h"

#define SET_BIT(var,pos) (var |= (1U << pos))
#define CLEAR_BIT(var,pos) (var &= ~(1U << pos))
#define COLOR_MATCH_THRESHOLD 10

CaptureHolder::CaptureHolder(QPoint point, QColor targetColor, QColor displayColor)
    : m_point(point)
    , m_targetColor(targetColor)
    , m_displayColor(displayColor)
    , m_mode(Mode::PointColorMatch)
{
    Register();
}

CaptureHolder::CaptureHolder(QPoint point, HsvRange range, QColor displayColor)
    : m_point(point)
    , m_range(range)
    , m_displayColor(displayColor)
    , m_mode(Mode::PointRangeMatch)
{
    Register();
}

CaptureHolder::CaptureHolder(QRect rect, QColor targetColor, QColor displayColor)
    : m_rect(rect)
    , m_targetColor(targetColor)
    , m_displayColor(displayColor)
    , m_mode(Mode::AreaColorMatch)
{
    Register();
}

CaptureHolder::CaptureHolder(QRect rect, HsvRange range, QColor displayColor)
    : m_rect(rect)
    , m_range(range)
    , m_displayColor(displayColor)
    , m_mode(Mode::AreaRangeMatch)
{
    Register();
}

CaptureHolder::~CaptureHolder()
{
    Unregister();
}

void CaptureHolder::SetArea(QRect rect)
{
    QMutexLocker locker(&m_mutex);
    m_rect = rect;
}

void CaptureHolder::SetPoint(QPoint point)
{
    QMutexLocker locker(&m_mutex);
    m_point = point;
}

void CaptureHolder::SetHsvRange(HsvRange range)
{
    QMutexLocker locker(&m_mutex);
    m_range = range;
}

void CaptureHolder::PushFrameData(const QImage &frame)
{
    // frame should already be in 1280x720
    // this is called by VLC thread
    QMutexLocker locker(&m_mutex);
    switch (m_mode)
    {
    case Mode::PointColorMatch:
    case Mode::PointRangeMatch:
    {
        m_testColor = frame.pixelColor(m_point);
        break;
    }
    case Mode::AreaColorMatch:
    case Mode::AreaRangeMatch:
    {
        m_testImage = frame.copy(m_rect);
        break;
    }
    }
}

QImage CaptureHolder::GetFrameData() const
{
    QMutexLocker locker(&m_mutex);
    return m_testImage.copy();
}

QColor CaptureHolder::GetPixelData() const
{
    QMutexLocker locker(&m_mutex);
    return m_testColor;
}

QRect CaptureHolder::GetRect() const
{
    QMutexLocker locker(&m_mutex);
    return m_rect;
}

QPoint CaptureHolder::GetPoint() const
{
    QMutexLocker locker(&m_mutex);
    return m_point;
}

HsvRange CaptureHolder::GetHsvRange() const
{
    QMutexLocker locker(&m_mutex);
    return m_range;
}

bool CaptureHolder::GetResultMatched() const
{
    QMutexLocker locker(&m_resultMutex);
    return m_resultMatched;
}

qreal CaptureHolder::GetResultMean() const
{
    QMutexLocker locker(&m_resultMutex);
    return m_resultMean;
}

QImage CaptureHolder::GetResultMasked() const
{
    QMutexLocker locker(&m_resultMutex);
    return m_resultMasked.copy();
}

bool CaptureHolder::GetColorMatchHSV(QColor testColor, HsvRange range)
{
    testColor = testColor.toHsv();

    // Test value and saturation first
    bool matched = testColor.value() >= range.min().value() && testColor.value() <= range.max().value()
                   && testColor.hsvSaturation() >= range.min().hsvSaturation() && testColor.hsvSaturation() <= range.max().hsvSaturation();

    // For achromatic colors it should be filltered in saturation and value
    if (matched && testColor.hsvHue() != -1)
    {
        int h = testColor.hsvHue();
        int h0 = range.min().hsvHue();
        int h1 = range.max().hsvHue();

        if (h0 > h1)
        {
            // 0-----------------359
            //     ^max     ^min
            //    <---        --->
            matched &= (h >= h0 || h <= h1);
        }
        else
        {
            // 0-----------------359
            //     ^max     ^min
            //       ---> <---
            matched &= (h >= h0 && h <= h1);
        }
    }

    return matched;
}

qreal CaptureHolder::GetBrightnessMean(const QImage &image, HsvRange range, QImage *masked)
{
    if (masked)
    {
        *masked = QImage(image.size(), QImage::Format_MonoLSB);
        masked->setColorTable({0xFF000000,0xFFFFFFFF});
    }

    double mean = 0;

    for (int y = 0; y < image.height(); y++)
    {
        QRgb *rowData = (QRgb*)image.scanLine(y);
        uint8_t *rowMaskedData = masked ? (uint8_t*)masked->scanLine(y) : Q_NULLPTR;
        for (int x = 0; x < image.width(); x++)
        {
            // Mask the target color
            bool matched = GetColorMatchHSV(QColor::fromRgb(rowData[x]), range);
            if (matched)
            {
                mean += 1;
            }

            if (rowMaskedData)
            {
                matched ? SET_BIT(rowMaskedData[x / 8], x % 8) : CLEAR_BIT(rowMaskedData[x / 8], x % 8);
            }
        }
    }

    // Get average value of brightness
    mean /= (image.height() * image.width());
    return mean;
}

void CaptureHolder::Register()
{
    ManagerCollection::GetManager<VideoManager>()->RegisterCapture(this);
}

void CaptureHolder::Unregister()
{
    ManagerCollection::GetManager<VideoManager>()->UnregisterCapture(this);
}
