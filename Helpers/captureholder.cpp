#include "captureholder.h"

#include "Helpers/jsonhelper.h"
#include "Managers/managercollection.h"
#include "Managers/videomanager.h"

#include <mutex>

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

CaptureHolder::CaptureHolder(QRect rect, HsvRange range, qreal mean, QColor displayColor)
    : m_rect(rect)
    , m_range(range)
    , m_targetMean(mean)
    , m_displayColor(displayColor)
    , m_mode(Mode::AreaRangeMatch)
{
    Register();
}

CaptureHolder::CaptureHolder(const QString &preset, QColor displayColor)
    : m_preset(preset)
    , m_displayColor(displayColor)
{
    // read preset json
    QString const name = GetDirectory() + m_preset + GetFormat();
    QJsonObject const object = JsonHelper::ReadJson(name);
    if (object.isEmpty()) return;

    QVariant value;
    if (!JsonHelper::ReadValue(object, "Mode", value)) return;
    Mode mode = (Mode)value.toInt();

    if (!JsonHelper::ReadValue(object, "Left", value)) return;
    m_point.setX(value.toInt());

    if (!JsonHelper::ReadValue(object, "Top", value)) return;
    m_point.setY(value.toInt());

    if (mode == Mode::AreaColorMatch || mode == Mode::AreaRangeMatch)
    {
        QSize size;
        if (!JsonHelper::ReadValue(object, "Width", value)) return;
        size.setWidth(value.toInt());

        if (!JsonHelper::ReadValue(object, "Height", value)) return;
        size.setHeight(value.toInt());

        m_rect = QRect(m_point, size);
    }

    if (mode == Mode::PointRangeMatch || mode == Mode::AreaRangeMatch)
    {
        int minH, minS, minV, maxH, maxS, maxV;
        if (!JsonHelper::ReadValue(object, "MinH", value)) return;
        minH = value.toInt();
        if (!JsonHelper::ReadValue(object, "MinS", value)) return;
        minS = value.toInt();
        if (!JsonHelper::ReadValue(object, "MinV", value)) return;
        minV = value.toInt();
        if (!JsonHelper::ReadValue(object, "MaxH", value)) return;
        maxH = value.toInt();
        if (!JsonHelper::ReadValue(object, "MaxS", value)) return;
        maxS = value.toInt();
        if (!JsonHelper::ReadValue(object, "MaxV", value)) return;
        maxV = value.toInt();

        m_range = HsvRange(minH, minS, minV, maxH, maxS, maxV);
    }

    if (mode == Mode::PointColorMatch || mode == Mode::AreaColorMatch)
    {
        if (!JsonHelper::ReadValue(object, "Color", value)) return;
        m_targetColor = QColor(value.toUInt());
    }

    if (mode == Mode::AreaRangeMatch)
    {
        if (!JsonHelper::ReadValue(object, "Mean", value)) return;
        m_targetMean = value.toDouble();
    }

    // if everything goes right then we finally set mode
    m_mode = mode;
    if (m_mode != Mode::Invalid)
    {
        Register();
    }
}

CaptureHolder::~CaptureHolder()
{
    Unregister();
}

void CaptureHolder::SetArea(QRect rect)
{
    std::unique_lock lock(m_mutex);
    m_rect = rect;
}

void CaptureHolder::SetPoint(QPoint point)
{
    std::unique_lock lock(m_mutex);
    m_point = point;
}

void CaptureHolder::SetTargetColor(QColor target)
{
    std::unique_lock lock(m_mutex);
    m_targetColor = target;
}

void CaptureHolder::SetTargetMean(qreal mean)
{
    std::unique_lock lock(m_mutex);
    m_targetMean = mean;
}

void CaptureHolder::SetHsvRange(HsvRange range)
{
    std::unique_lock lock(m_mutex);
    m_range = range;
}

void CaptureHolder::PushFrameData(const QImage &frame)
{
    QMutexLocker locker(&m_workMutex);
    if (m_pendingWork) return;

    // frame should already be in 1280x720
    // this is called by VLC thread
    std::unique_lock lock(m_mutex);
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
    default: break;
    }

    m_pendingWork = true;
    m_condition.wakeOne();
}

QImage CaptureHolder::GetFrameData() const
{
    std::shared_lock lock(m_mutex);
    return m_testImage.copy();
}

QColor CaptureHolder::GetPixelData() const
{
    std::shared_lock lock(m_mutex);
    return m_testColor;
}

QRect CaptureHolder::GetRect() const
{
    std::shared_lock lock(m_mutex);
    return m_rect;
}

QPoint CaptureHolder::GetPoint() const
{
    std::shared_lock lock(m_mutex);
    return m_point;
}

QColor CaptureHolder::GetTargetColor() const
{
    std::shared_lock lock(m_mutex);
    return m_targetColor;
}

HsvRange CaptureHolder::GetHsvRange() const
{
    std::shared_lock lock(m_mutex);
    return m_range;
}

bool CaptureHolder::GetResultMatched() const
{
    std::shared_lock lock(m_resultMutex);
    return m_resultMatched;
}

qreal CaptureHolder::GetResultMean() const
{
    std::shared_lock lock(m_resultMutex);
    return m_resultMean;
}

QColor CaptureHolder::GetResultColor() const
{
    std::shared_lock lock(m_resultMutex);
    return m_resultColor;
}

QImage CaptureHolder::GetResultMasked() const
{
    std::shared_lock lock(m_resultMutex);
    return m_resultMasked.copy();
}

QString CaptureHolder::GetResultString() const
{
    std::shared_lock lock(m_resultMutex);
    return m_resultString;
}

bool CaptureHolder::GetColorMatch(QColor testColor, QColor target)
{
    int const r = target.red() - testColor.red();
    int const g = target.green() - testColor.green();
    int const b = target.blue() - testColor.blue();
    return r*r + g*g + b*b <= COLOR_MATCH_THRESHOLD * COLOR_MATCH_THRESHOLD;
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
        int const h = testColor.hsvHue();
        int const h0 = range.min().hsvHue();
        int const h1 = range.max().hsvHue();

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

QColor CaptureHolder::GetAverageColor(const QImage &image)
{
    qreal r = 0;
    qreal g = 0;
    qreal b = 0;
    for (int y = 0; y < image.height(); y++)
    {
        QRgb const* rowData = (QRgb*)image.scanLine(y);
        for (int x = 0; x < image.width(); x++)
        {
            QColor const color = QColor::fromRgb(rowData[x]);
            r += color.redF();
            g += color.greenF();
            b += color.blueF();
        }
    }

    qreal const pixelCount = image.height() * image.width();
    r /= pixelCount;
    g /= pixelCount;
    b /= pixelCount;

    QColor testColor;
    testColor.setRgbF(r,g,b);
    return testColor;
}

bool CaptureHolder::GetAverageColorMatch(const QImage &image, QColor target)
{
    QColor const testColor = GetAverageColor(image);
    return GetColorMatch(testColor, target);
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
        QRgb const* rowData = (QRgb*)image.scanLine(y);
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
