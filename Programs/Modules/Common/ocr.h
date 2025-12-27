#ifndef OCR_H
#define OCR_H

#include <QFile>
#include <QImage>
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

    // from ModuleBase
    QString GetName() const override { return "Common-OCR"; }

    // from QThread
    void run() override;

private slots:
    void OnProcessErrored(QProcess::ProcessError error);
    void OnProcessFinished();

private:
    QProcess    m_process;
    QImage      m_image;
    bool        m_isNumber = false;
    bool        m_shouldInvert = true;
    LanguageType m_language = LT_English;

    // result
    QString     m_resultRawString;

};
}

#endif // OCR_H
