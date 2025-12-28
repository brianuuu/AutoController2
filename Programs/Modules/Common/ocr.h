#ifndef OCR_H
#define OCR_H

#include <QFile>
#include <QImage>
#include <QMetaEnum>
#include <QProcess>

#include "../modulebase.h"
#include "Types/languagetype.h"

namespace Module::Common
{
class OCR : public ModuleBase
{
    Q_OBJECT
public:
    explicit OCR(QImage const& image, bool isNumber, bool shouldInvert = true, QObject *parent = nullptr);
    void SetOCREntries(QString const& database) { m_database = database; }

    // from ModuleBase
    QString GetName() const override { return "Common-OCR"; }

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

private:
    QProcess    m_process;
    QImage      m_image;
    bool        m_isNumber = false;
    bool        m_shouldInvert = true;
    LanguageType m_language = LT_English;
    QString     m_database;

    // result
    QString     m_resultEntry;
    QString     m_resultRawString;
    int         m_resultNumber;

};
}

#endif // OCR_H
