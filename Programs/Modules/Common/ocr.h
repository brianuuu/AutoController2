#ifndef OCR_H
#define OCR_H

#include <QFile>
#include <QImage>
#include <QMetaEnum>
#include <QProcess>
#include <QWaitCondition>

#include "../modulebase.h"
#include "Helpers/captureholder.h"
#include "Types/languagetype.h"

namespace Module::Common
{
class OCR : public ModuleBase, public CaptureHolder
{
    Q_OBJECT
public:
    explicit OCR(QImage const& image, bool isNumber = false, bool shouldInvert = true);
    explicit OCR(QString const& preset, QString const& database, bool isNumber = false, QColor displayColor = QColor(0,255,0));
    void SetOCREntries(QString const& database) { m_database = database; }

    // from ModuleBase
    QString GetName() const override { return "Common-OCR"; }

    // from CaptureHolder
    void PushFrameData(QImage const& frame) override;

    // from QThread
    void run() override;

    QString GetResultEntry() const { return m_resultEntry; }
    QString GetResultRawString() const { return m_resultRawString; }
    int GetResultNumber() const { return m_resultNumber; }

private slots:
    void OnProcessErrored(QProcess::ProcessError error);
    void OnProcessFinished();

private:
    QString GetCaptureName() const { return "capture" + QString::number(GetID()) + ".png"; }
    QString GetOutputName() const { return "output" + QString::number(GetID()); }
    QString GetOutputTextName() const { return GetOutputName() + ".txt"; }

    void Init();

private:
    QProcess    m_process;
    QImage      m_image;
    bool        m_isNumber = false;
    bool        m_shouldInvert = true;
    LanguageType m_language = LT_English;

    // frame capture
    QString     m_preset;
    QString     m_database;
    QWaitCondition  m_condition;
    mutable QMutex  m_workMutex;
    bool        m_pendingWork = false;

    // result
    QString     m_resultEntry;
    QString     m_resultRawString;
    int         m_resultNumber;

};
}

#endif // OCR_H
