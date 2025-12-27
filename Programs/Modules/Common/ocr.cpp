#include "ocr.h"

#include "Managers/managercollection.h"
#include "Managers/profilemanager.h"

#define OCR_DIRECTORY QString("../Resources/Tesseract/")

namespace Module::Common
{

OCR::OCR(const QImage &image, bool isNumber, bool shouldInvert, QObject *parent)
    : ModuleBase(parent)
    , m_image(image)
    , m_isNumber(isNumber)
    , m_shouldInvert(shouldInvert)
{
    ProfileManager* profileManager = ManagerCollection::GetManager<ProfileManager>();
    m_language = profileManager->GetLanguageType();

    m_process.moveToThread(this);
    connect(&m_process, &QProcess::errorOccurred, this, &OCR::OnProcessErrored);
    connect(&m_process, &QProcess::finished, this, &OCR::OnProcessFinished);
}

void OCR::run()
{
    // by default, image from FrameCapture has black BG white text
    // OCR works better having white BG black text
    if (m_shouldInvert)
    {
        m_image.invertPixels();
    }

    m_image.save(OCR_DIRECTORY + "capture.png", "PNG");

    // Check if .traineddata exist
    if (!ProfileManager::OcrTrainedDataExist(m_language))
    {
        QString const languageName = LanguageToString(m_language);
        m_error = "Language trained data for '" + languageName + "' for Tesseract is missing, please goto 'Resources/Tesseract' folder and follow the instructions in README.md";
        m_result = -1;
        return;
    }

    QString command = OCR_DIRECTORY + "tesseract.exe ";
    command += "./capture.png ./output --tessdata-dir . ";
    command += "-l " + LanguageToPrefix(m_language);
    if (m_language == LT_ChineseSimplified || m_language == LT_ChineseTraditional || m_language == LT_Korean)
    {
        command += " --psm 7 --oem 0 -c tessedit_create_txt=1";
    }
    else
    {
        command += " --psm 7 --oem 2 -c tessedit_create_txt=1";
    }
    m_process.setWorkingDirectory(OCR_DIRECTORY);
    m_process.start(command);

    // wait for process signals
    exec();
    if (m_result < 0 || m_terminate) return;

    // get raw string
    QFile output(OCR_DIRECTORY + "output.txt");
    if (output.open(QIODevice::Text | QIODevice::ReadOnly))
    {
        QTextStream in(&output);
        in.setEncoding(QStringConverter::Utf8);
        m_resultRawString = in.readLine();
        output.close();
    }
    else
    {
        m_error = "Unable to open output.txt";
        m_result = -1;
        return;
    }

    // TODO: entry matching
}

void OCR::OnProcessErrored(QProcess::ProcessError error)
{
    m_error = "Unable to start text recognition, tesseract.exe might be missing.\nProcess exited with code: " + QString::number(error);
    m_result = -1;
    quit();
}

void OCR::OnProcessFinished()
{
    if (!QFile::exists(OCR_DIRECTORY + "output.txt"))
    {
        m_error = "Expected tesseract output.txt not found";
        m_result = -1;
    }
    quit();
}

}
