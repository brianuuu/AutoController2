#ifndef FRAMECAPTURE_H
#define FRAMECAPTURE_H

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
    explicit FrameCapture(QString const& preset, QColor displayColor = QColor(0,255,0), QObject *parent = nullptr);

    // from ModuleBase
    QString GetName() const override { return "Common-FrameCapture"; }
    void stop() override;

    // from QThread
    void run() override;

signals:
    void notifyResultMatched(Module::Common::FrameCapture*, bool);
    void notifyResultMean(qreal, QImage);
};

}

#endif // FRAMECAPTURE_H
