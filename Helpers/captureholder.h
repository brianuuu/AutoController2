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
    CaptureHolder(QRect rect, HsvRange range, QColor color = QColor(0,255,0));
    CaptureHolder(QPoint point, QColor color = QColor(0,255,0));
    ~CaptureHolder();

    void SetArea(QRect rect);
    void SetPoint(QPoint point);
    void SetHsvRange(HsvRange range);

    // get data for analysis
    virtual void PushFrameData(QImage const& frame);
    QImage GetFrameData() const;
    QColor GetPixelData() const;

    // get fixed data
    QRect GetRect() const;
    QPoint GetPoint() const;
    HsvRange GetHsvRange() const;
    bool GetIsArea() const { return m_isArea; }
    QColor GetDisplayColor() const { return m_displayColor; }

    // analysis
    static QSize GetCaptureResolution() { return QSize(1280,720); }
    static bool GetColorMatchHSV(QColor testColor, HsvRange range);
    static qreal GetBrightnessMean(QImage const& image, HsvRange range, QImage* masked = Q_NULLPTR);

private:
    void Register();
    void Unregister();

private:
    mutable QMutex  m_mutex;
    bool            m_isArea = true;
    QRect           m_rect;
    QPoint          m_point;
    HsvRange        m_range;
    QColor          m_displayColor;

    QImage  m_testImage;
    QColor  m_testColor;
};

#endif // CAPTUREHOLDER_H
