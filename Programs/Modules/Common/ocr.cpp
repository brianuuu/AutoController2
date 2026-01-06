#include "ocr.h"

#include "Helpers/ocrentrydatabase.h"
#include "Managers/managercollection.h"
#include "Managers/profilemanager.h"
#include "defines.h"

namespace Module::Common
{

OCR::OCR(const QImage &image, bool isNumber, bool shouldInvert)
    : m_image(image)
    , m_isNumber(isNumber)
    , m_shouldInvert(shouldInvert)
{
    Init();
}

OCR::OCR(const QString &preset, const QString &database, bool isNumber, QColor displayColor)
    : CaptureHolder(preset, displayColor)
    , m_isNumber(isNumber)
    , m_shouldInvert(true)
    , m_preset(preset)
    , m_database(database)
{
    Init();
}

void OCR::Init()
{
    ProfileManager* profileManager = ManagerCollection::GetManager<ProfileManager>();
    m_language = profileManager->GetLanguageType();

    m_process.moveToThread(this);
    connect(&m_process, &QProcess::errorOccurred, this, &OCR::OnProcessErrored);
    connect(&m_process, &QProcess::finished, this, &OCR::OnProcessFinished);
}

void OCR::PushFrameData(const QImage &frame)
{
    if (!isRunning()) return;

    QMutexLocker locker(&m_workMutex);
    if (m_pendingWork) return;

    CaptureHolder::PushFrameData(frame);

    m_pendingWork = true;
    m_condition.wakeOne();
}

void OCR::run()
{
    // if it has preset, do frame capture ourself
    if (!m_preset.isEmpty())
    {
        if (m_mode == Mode::Invalid)
        {
            m_error = "Preset \"" + m_preset + "\" does not exist";
            m_result = -1;
            return;
        }

        PrintLog("Detecting with preset \"" + m_preset + "\"");
        QImage frame;
        {
            // wait for work
            QMutexLocker workLocker(&m_workMutex);
            while (!m_pendingWork && !m_terminate)
            {
                m_condition.wait(&m_workMutex);
            }

            if (m_terminate) return;
            frame = GetFrameData();
        }

        // Note: this never reset m_pendingWork = false since we only need it once
        std::unique_lock lock(m_resultMutex);
        HsvRange const range = GetHsvRange();
        m_resultMean = GetBrightnessMean(frame, range, &m_resultMasked);
        m_image = m_resultMasked.copy();
    }

    if (m_image.isNull())
    {
        m_error = "No image provided";
        m_result = -1;
        return;
    }

    // last chance to terminate
    if (m_result < 0 || m_terminate) return;

    // by default, image from FrameCapture has black BG white text
    // OCR works better having white BG black text
    if (m_shouldInvert)
    {
        m_image.invertPixels();
    }

    m_image.save(OCR_PATH + GetCaptureName(), "PNG");

    // Check if .traineddata exist
    if (!ProfileManager::OCRTrainedDataExist(m_language))
    {
        m_error = ProfileManager::GetNoTrainedDataError(m_language);
        m_result = -1;
        return;
    }

    QStringList arg;
    arg << "./" + GetCaptureName();
    arg << "./" + GetOutputName();
    arg << "--tessdata-dir" << ".";
    arg << "-l" << LanguageToPrefix(m_language);
    arg << "--psm" << "7";
    if (m_language == LT_ChineseSimplified || m_language == LT_ChineseTraditional || m_language == LT_Korean)
    {
        arg << "--oem" << "0";
    }
    else
    {
        arg << "--oem" << "2";
    }
    arg << "-c" << "tessedit_create_txt=1";
    m_process.setWorkingDirectory(OCR_PATH);
    m_process.start(OCR_PATH + "tesseract.exe", arg);

    // wait for process signals
    exec();
    if (m_result < 0 || m_terminate)
    {
        m_process.waitForFinished(-1);
        return;
    }

    // get raw string
    QFile output(OCR_PATH + GetOutputTextName());
    if (output.open(QIODevice::Text | QIODevice::ReadOnly))
    {
        QTextStream in(&output);
        in.setEncoding(QStringConverter::Utf8);
        m_resultRawString = in.readLine();
        output.close();
    }
    else
    {
        m_error = "Unable to open " + GetOutputTextName();
        m_result = -1;
        return;
    }

    // remove junk character
    m_resultRawString.remove((char)0xC);
    if (m_resultRawString.isEmpty())
    {
        PrintLog("OCR returned nothing", LOG_Warning);
        m_result = 1;
        return;
    }

    if (m_isNumber)
    {
        QString numStr;
        bool hasDigit = false;
        for (QChar ch : std::as_const(m_resultRawString))
        {
            // in case it false detect as these...?
            if (ch == 'l' || ch == 'i' || ch == 't' || ch == 'F' || ch == 'f')
            {
                PrintLog(QString("Letter '") + ch + "' is converted to 1", LOG_Warning);
                ch = '1';
            }

            if (ch.isDigit())
            {
                numStr += ch;
                hasDigit = true;
            }
        }

        if (!hasDigit)
        {
            PrintLog("OCR failed to read any number", LOG_Warning);
            m_result = 1;
            return;
        }

        // This should be number but we check convertion just in case
        bool ok = false;
        m_resultNumber = numStr.toInt(&ok);

        if (ok)
        {
            PrintLog("OCR returned number: " + numStr);
        }
        else
        {
            PrintLog("OCR failed to convert number", LOG_Warning);
            m_result = 1;
            return;
        }
    }
    else
    {
        PrintLog("OCR returned string: " + m_resultRawString);
    }

    // no entry match work
    if (m_database.isEmpty()) return;

    // entry matching
    OCREntries const& entries = OCREntryDatabase::GetEntries(m_database, m_language);
    if (entries.empty())
    {
        PrintLog(OCREntryDatabase::GetNoDatabaseError(m_database, m_language), LOG_Error);
        m_result = -1;
        return;
    }

    QString const query = OCREntryDatabase::NormalizeString(m_resultRawString);

    // Do comparison with each database string, find the best match entry
    int minDist = INT_MAX;
    int minSubStringMatched = -1;
    for (auto const& [entry, subStrings] : entries)
    {
        int dist = 0;
        int subStringMatched = OCREntryDatabase::MatchSubStrings(query, subStrings, &dist);
        if (subStringMatched >= 0 && subStringMatched < subStrings.size() && dist < minDist)
        {
            minDist = dist;
            m_resultEntry = entry;
            minSubStringMatched = subStringMatched;
        }
    }

    if (m_resultEntry.isEmpty())
    {
        PrintLog("OCR text \"" + m_resultRawString + "\" found no matches", LOG_Warning);
        m_result = 1;
    }
    else
    {
        PrintLog("OCR text \"" + m_resultRawString + "\" has matched entry \"" + m_resultEntry + "\" (LD = " + QString::number(minDist) + ")");
    }
}

void OCR::OnProcessErrored(QProcess::ProcessError error)
{
    m_error = QString("Unable to start text recognition, tesseract.exe might be missing.\nProcess exited with: ") + QMetaEnum::fromType<QProcess::ProcessError>().valueToKey(error);
    m_result = -1;
    quit();
}

void OCR::OnProcessFinished()
{
    if (!QFile::exists(OCR_PATH + GetOutputTextName()))
    {
        m_error = "Expected tesseract " + GetOutputTextName() + " not found";
        m_result = -1;
    }
    quit();
}

}
