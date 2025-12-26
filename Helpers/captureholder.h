#ifndef CAPTUREHOLDER_H
#define CAPTUREHOLDER_H

#include <qcolor.h>
#include <qimage.h>
#include <qmutex.h>
#include <qrect.h>
#include <qpoint.h>

struct HsvRange
{
    HsvRange()
    {
        m_minHSV.setHsv(0,0,0);
        m_maxHSV.setHsv(359,255,255);
    }
    HsvRange(int minH, int minS, int minV, int maxH, int maxS, int maxV)
    {
        m_minHSV.setHsv(minH,minS,minV);
        m_maxHSV.setHsv(maxH,maxS,maxV);
    }
    HsvRange(QColor minHSV, QColor maxHSV)
    {
        Q_ASSERT(minHSV.spec() == QColor::Hsv);
        Q_ASSERT(maxHSV.spec() == QColor::Hsv);
        m_minHSV = minHSV;
        m_maxHSV = maxHSV;
    }

    QColor min() const {return m_minHSV;}
    QColor max() const {return m_maxHSV;}

private:
    QColor m_minHSV;
    QColor m_maxHSV;
};

class CaptureHolder
{
public:
    enum class Mode {
        PointColorMatch,
        PointRangeMatch,
        AreaColorMatch,
        AreaRangeMatch,
    };

public:
    CaptureHolder(QPoint point, QColor targetColor, QColor displayColor = QColor(0,255,0));
    CaptureHolder(QPoint point, HsvRange range, QColor displayColor = QColor(0,255,0));
    CaptureHolder(QRect rect, QColor targetColor, QColor color = QColor(0,255,0));
    CaptureHolder(QRect rect, HsvRange range, QColor color = QColor(0,255,0));
    ~CaptureHolder();

    // get innt data
    Mode GetMode() const { return m_mode; }
    QColor GetDisplayColor() const { return m_displayColor; }

    // set fixed data
    void SetArea(QRect rect);
    void SetPoint(QPoint point);
    void SetTargetColor(QColor target);
    void SetHsvRange(HsvRange range);

    // get data for analysis
    virtual void PushFrameData(QImage const& frame);
    QImage GetFrameData() const;
    QColor GetPixelData() const;

    // get fixed data
    QRect GetRect() const;
    QPoint GetPoint() const;
    QColor GetTargetColor() const;
    HsvRange GetHsvRange() const;

    // analysis
    static QSize GetCaptureResolution() { return QSize(1280,720); }
    static bool GetColorMatch(QColor testColor, QColor target);
    static bool GetColorMatchHSV(QColor testColor, HsvRange range);
    static bool GetAverageColorMatch(QImage const& image, QColor target);
    static qreal GetBrightnessMean(QImage const& image, HsvRange range, QImage* masked = Q_NULLPTR);

    // results
    bool GetResultMatched() const;
    qreal GetResultMean() const;
    QImage GetResultMasked() const;

private:
    void Register();
    void Unregister();

protected:
    // init data
    QColor  m_displayColor;
    Mode    m_mode;

    mutable QMutex  m_mutex;
    // fixed data
    QRect       m_rect;
    QPoint      m_point;
    QColor      m_targetColor;
    HsvRange    m_range;

    // frame data
    QImage  m_testImage;
    QColor  m_testColor;

    // results
    mutable QMutex  m_resultMutex;
    bool    m_resultMatched = false;
    qreal   m_resultMean = 0.0;
    QImage  m_resultMasked;
};

#endif // CAPTUREHOLDER_H
