#include "logmanager.h"

#include "../ui_mainwindow.h"
#include "Helpers/jsonhelper.h"

#define LOG_PATH "../Logs/"

void LogManager::Initialize(Ui::MainWindow *ui)
{
    if (!QDir(LOG_PATH).exists())
    {
        QDir().mkdir(LOG_PATH);
    }

    connect(ui->PB_OutputWindow, &QPushButton::clicked, this, &LogManager::OnShow);

    // Setup layout
    this->setWindowTitle("Output Log");
    this->resize(640,480);

    QVBoxLayout* vBoxLayout = new QVBoxLayout(this);

    m_browser = new QTextBrowser();
    vBoxLayout->addWidget(m_browser);

    QHBoxLayout* hBoxLayout = new QHBoxLayout();
    vBoxLayout->addLayout(hBoxLayout);

    m_btnSave = new QPushButton("Save Log");
    hBoxLayout->addWidget(m_btnSave);
    connect(m_btnSave, &QPushButton::clicked, this, &LogManager::OnSaveLog);

    m_btnClear = new QPushButton("Clear Log");
    hBoxLayout->addWidget(m_btnClear);
    connect(m_btnClear, &QPushButton::clicked, this, &LogManager::OnClearLog);

    OnClearLog();
    LoadSettings();
}

void LogManager::closeEvent(QCloseEvent *event)
{
    // triggers when user closes this window
    m_defaultShow = false;
    SaveSettings();
    QWidget::closeEvent(event);
}

bool LogManager::OnCloseEvent()
{
    // triggers when main window closes, don't change m_defaultShow
    SaveSettings();
    this->hide();
    return true;
}

bool LogManager::OnInitShow()
{
    if (m_defaultShow && this->isHidden())
    {
        OnShow();
        return true;
    }

    return false;
}

void LogManager::OnShow()
{
    m_defaultShow = true;
    this->show();
    if (this->isMinimized())
    {
        this->showNormal();
    }
    this->activateWindow();
}

void LogManager::LoadSettings()
{
    QJsonObject settings = JsonHelper::ReadSetting("LogWindow");
    {
        QJsonObject windowSize = JsonHelper::ReadObject(settings, "WindowSize");

        QVariant x, y;
        if (JsonHelper::ReadValue(windowSize, "X", x) && JsonHelper::ReadValue(windowSize, "Y", y))
        {
            this->move(x.toInt(), y.toInt());
        }

        QVariant width, height;
        if (JsonHelper::ReadValue(windowSize, "Width", width) && JsonHelper::ReadValue(windowSize, "Height", height))
        {
            this->resize(width.toInt(), height.toInt());
        }
    }
    {
        QVariant defaultShow;
        if (JsonHelper::ReadValue(settings, "DefaultShow", defaultShow))
        {
            m_defaultShow = defaultShow.toBool();
        }
    }
}

void LogManager::SaveSettings() const
{
    QJsonObject windowSize;
    windowSize.insert("Width", this->width());
    windowSize.insert("Height", this->height());
    windowSize.insert("X", this->pos().x());
    windowSize.insert("Y", this->pos().y());

    QJsonObject settings;
    settings.insert("WindowSize", windowSize);
    settings.insert("DefaultShow", m_defaultShow);

    JsonHelper::WriteSetting("LogWindow", settings);
}

void LogManager::SetCurrentLogFile(const QString &file)
{
    m_logFile = file;
}

void LogManager::SetClearLogEnabled(bool enable)
{
    m_btnClear->setEnabled(enable);
}

QString LogManager::GetLogFileName(const QString &name)
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + "_" + name + ".log";
}

void LogManager::PrintLog(const QString &category, const QString &log, LogType type)
{
    QString const header = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") + " - [" + category + "]";
    QColor const color = LogTypeToColor(type);
    QString r,g,b;
    r.setNum(color.red(), 16); if (color.red() < 0x10) r = "0" + r;
    g.setNum(color.green(), 16); if (color.green() < 0x10) g = "0" + g;
    b.setNum(color.blue(), 16); if (color.blue() < 0x10) b = "0" + b;
    QString const html = "<font color=\"#FF" + r + g + b + "\">" + header + " " + log + "</font>";

    m_logCount++;
    m_browser->append(html);

    if (!m_logFile.isEmpty())
    {
        QFile file(m_logFile);
        if(file.open(QIODevice::Append))
        {
            QTextStream stream(&file);
            stream << header + LogTypeDisplayText(type) + " " + log + "\n";
            file.close();
        }
    }

    // Clear if there are too many logs
    if (m_logCount >= 10000)
    {
        OnClearLog();
    }
}

void LogManager::OnSaveLog()
{
    QString const nameWithTime = GetLogFileName("Log");
    QFile file(LOG_PATH + nameWithTime);
    if(file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        stream << m_browser->toPlainText();
        file.close();

        PrintLog("Global", "Log file saved: " + nameWithTime);
    }
}

void LogManager::OnClearLog()
{
    m_logCount = 0;
    m_browser->clear();
}
