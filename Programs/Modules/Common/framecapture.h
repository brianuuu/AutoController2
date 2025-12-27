#ifndef FRAMECAPTURE_H
#define FRAMECAPTURE_H

#include <QWaitCondition>

#include "../modulebase.h"
#include "Helpers/captureholder.h"

namespace Module::Common
{
class FrameCapture : public ModuleBase, public CaptureHolder
{
    Q_OBJECT
public:
    explicit FrameCapture(QPoint point, QColor testColor, QColor displayColor = QColor(0,255,0), QObject *parent = nullptr);
    explicit FrameCapture(QPoint point, HsvRange range, QColor displayColor = QColor(0,255,0), QObject *parent = nullptr);
    explicit FrameCapture(QRect rect, QColor testColor, QColor displayColor = QColor(0,255,0), QObject *parent = nullptr);
    explicit FrameCapture(QRect rect, HsvRange range, QColor displayColor = QColor(0,255,0), QObject *parent = nullptr);

    // from ModuleBase
    QString GetName() const override { return "Common-FrameCapture"; }
    void stop() override;

    // from CaptureHolder
    void PushFrameData(QImage const& frame) override;

    // from QThread
    void run() override;

private:
    QWaitCondition  m_condition;
    mutable QMutex  m_workMutex;
    bool    m_pendingWork = false;
};

}

#endif // FRAMECAPTURE_H
