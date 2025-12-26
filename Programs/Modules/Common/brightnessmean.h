#ifndef BRIGHTNESSMEAN_H
#define BRIGHTNESSMEAN_H

#include <QWaitCondition>

#include "../modulebase.h"
#include "Helpers/captureholder.h"

namespace Module::Common
{
class BrightnessMean : public ModuleBase, public CaptureHolder
{
     Q_OBJECT

public:
    explicit BrightnessMean
    (
        QRect rect,
        HsvRange range,
        QColor color = QColor(0,255,0),
        QObject *parent = nullptr
    );

    // from ModuleBase
    QString GetName() const override { return "Common-BrightnessMean"; }
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

#endif // BRIGHTNESSMEAN_H
