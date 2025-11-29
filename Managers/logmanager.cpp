#include "logmanager.h"

#define LOG_PATH "../Logs/"

LogManager::LogManager
(
    QWidget *parent
)
    : QWidget(parent)
{
    if (!QDir(LOG_PATH).exists())
    {
        QDir().mkdir(LOG_PATH);
    }

    // Setup layout
    this->setWindowTitle("Output Log");
    this->resize(640,480);

    QVBoxLayout* vBoxLayout = new QVBoxLayout(this);

    m_browser = new QTextBrowser();
    vBoxLayout->addWidget(m_browser);

    m_btnClear = new QPushButton("Clear Log");
    vBoxLayout->addWidget(m_btnClear);
    connect(m_btnClear, &QPushButton::clicked, m_browser, [this]{ ClearLog(); });

    ClearLog();
    this->show();
}

bool LogManager::OnCloseEvent()
{
    this->hide();
    return true;
}

void LogManager::SetCurrentLogFile(const QString &file)
{
    m_logFile = file;
}

void LogManager::SetClearLogEnabled(bool enable)
{
    m_btnClear->setEnabled(enable);
}

void LogManager::PrintLog(const QString &log)
{
    QString str = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") + " - " + log;

    m_logCount++;
    m_browser->append(str);

    if (!m_logFile.isEmpty())
    {
        QFile file(m_logFile);
        if(file.open(QIODevice::Append))
        {
            QTextStream stream(&file);
            stream << str + "\n";
            file.close();
        }
    }
}

void LogManager::ClearLog()
{
    m_logCount = 0;
    m_browser->clear();
}
