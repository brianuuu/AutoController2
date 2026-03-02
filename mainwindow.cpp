#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "Helpers/jsonhelper.h"
#include "Managers/discordmanager.h"
#include "Managers/joystickmanager.h"
#include "Managers/keyboardmanager.h"
#include "Managers/logmanager.h"
#include "Managers/profilemanager.h"
#include "Managers/programmanager.h"
#include "Managers/serialmanager.h"
#include "Managers/vlcmanager.h"
#include "defines.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QString title = "Auto Controller 2 v" + VERSION;
    if (IS_BETA)
    {
        title += " (Beta)";
    }
    this->setWindowTitle(title);

    m_logManager = ManagerCollection::AddManager<LogManager>();
    m_discordManager = ManagerCollection::AddManager<DiscordManager>();
    m_profileManager = ManagerCollection::AddManager<ProfileManager>();
    m_joystickManager = ManagerCollection::AddManager<JoystickManager>(this);
    m_keyboardManager = ManagerCollection::AddManager<KeyboardManager>();
    m_serialManager = ManagerCollection::AddManager<SerialManager>(this);
    m_vlcManager = ManagerCollection::AddManager<VlcManager>();
    m_programManager = ManagerCollection::AddManager<ProgramManager>(this);

    m_logManager->Initialize(ui);
    m_profileManager->Initialize(ui);
    m_joystickManager->Initialize(ui);
    m_keyboardManager->Initialize(ui);
    m_serialManager->Initialize(ui);
    m_vlcManager->Initialize(ui);
    m_programManager->Initialize(ui);

    LoadSettings();

    this->installEventFilter(this);
    m_logManager->PrintLog("Global", "Initialization completed");

    // check for update
    m_networkManager = new QNetworkAccessManager();
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &MainWindow::OnNetworkManagerFinished);
    m_networkRequest.setUrl(QUrl("https://raw.githubusercontent.com/brianuuu/AutoController2/refs/heads/main/build/UpdateVersion.ini"));
    m_networkManager->get(m_networkRequest);
    ui->L_Update->setText("Checking for Update...");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!m_programManager->OnCloseEvent())
    {
        event->ignore();
        return;
    }

    if (!m_vlcManager->OnCloseEvent())
    {
        event->ignore();
        return;
    }

    if (!m_serialManager->OnCloseEvent())
    {
        event->ignore();
        return;
    }

    if (!m_keyboardManager->OnCloseEvent())
    {
        event->ignore();
        return;
    }

    if (!m_profileManager->OnCloseEvent())
    {
        event->ignore();
        return;
    }

    if (!m_logManager->OnCloseEvent())
    {
        event->ignore();
        return;
    }

    SaveSettings();

    delete m_vlcManager;
    delete m_keyboardManager;
    delete m_profileManager;
    delete m_discordManager;
    delete m_logManager;

    m_vlcManager = Q_NULLPTR;
    m_keyboardManager = Q_NULLPTR;
    m_profileManager = Q_NULLPTR;
    m_discordManager = Q_NULLPTR;
    m_logManager = Q_NULLPTR;

    QMainWindow::closeEvent(event);
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    // a bit of a hack to make window appear after main window
    if (m_logManager->OnInitShow() || m_keyboardManager->OnInitShow())
    {
        // steal focus back
        this->activateWindow();
    }

    QMainWindow::paintEvent(event);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    QWidget* widget = qobject_cast<QWidget*>(watched);
    if (event->type() == QEvent::ActivationChange || event->type() == QEvent::WindowStateChange)
    {
        // when main window activates, raise other windows too
        if (widget->isActiveWindow())
        {
            if (m_logManager) m_logManager->raise();
            if (m_profileManager) m_profileManager->raise();
            if (m_keyboardManager) m_keyboardManager->raise();
            if (m_vlcManager) m_vlcManager->raise();
            this->raise();
        }

        if (m_keyboardManager)
        {
            m_keyboardManager->OnUpdateStatus();
        }
    }

    return false;
}

void MainWindow::LoadSettings()
{
    QJsonObject settings = JsonHelper::ReadSetting("MainWindow");
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
}

void MainWindow::SaveSettings() const
{
    QJsonObject windowSize;
    windowSize.insert("Width", this->width());
    windowSize.insert("Height", this->height());
    windowSize.insert("X", this->pos().x());
    windowSize.insert("Y", this->pos().y());

    QJsonObject settings;
    settings.insert("WindowSize", windowSize);

    JsonHelper::WriteSetting("MainWindow", settings);
}

void MainWindow::OnNetworkManagerFinished(QNetworkReply *reply)
{
    QString link = "https://github.com/brianuuu/AutoController2/releases";

    if (reply->error())
    {
        QString message = "Update check failed: " + reply->errorString();
        message += "\n\nDo you want to check the release page for newest version?";

        QMessageBox::StandardButton resBtn = QMessageBox::Yes;
        resBtn = QMessageBox::question(this, "Error", message, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        if (resBtn == QMessageBox::Yes)
        {
            QDesktopServices::openUrl(QUrl(link));
        }

        ui->L_Update->setText("Update Check Failed");
        return;
    }
    else if (m_lastestVersion.isEmpty())
    {
        QString const answer = reply->readAll();
        int const verStart = answer.indexOf("Version=\"") + 9;
        int const verEnd = answer.indexOf('\"', verStart);
        m_lastestVersion = answer.mid(verStart, verEnd - verStart);

        QStringList newVerNo = m_lastestVersion.split('.');
        QStringList curVerNo = VERSION.split('.');

        bool outdated = false;
        for (int i = 0; i < newVerNo.size(); i++)
        {
            if (newVerNo[i] > curVerNo[i])
            {
                outdated = true;
                break;
            }
            else if (curVerNo[i] > newVerNo[i])
            {
                // Program is newer than github, not commited yet
                ui->L_Update->setText("Github not commited.");
                return;
            }
        }

        if (outdated)
        {
            // get changelog data
            m_networkRequest.setUrl(QUrl("https://raw.githubusercontent.com/brianuuu/AutoController2/refs/heads/main/build/Changelogs/v" + m_lastestVersion + ".txt"));
            m_networkManager->get(m_networkRequest);
        }
        else
        {
            ui->L_Update->setText("Program Up to Date!");
        }
    }
    else
    {
        ui->L_Update->setText("<html><head/><body><p><a href=\"" + link + "\">Update Available!</span></a></p></body></html>");

        QString const changeLog = reply->readAll();
        QMessageBox::StandardButton resBtn = QMessageBox::Yes;
        QString message = "New version v" + m_lastestVersion + " available, do you wish to download it?";
        message += "\n\nChange Log:\n" + changeLog;
        resBtn = QMessageBox::question(this, "Update", message, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        if (resBtn == QMessageBox::Yes)
        {
            QDesktopServices::openUrl(QUrl(link));
            this->close();
        }
    }
}
