#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <qmediadevices.h>
#include <QCameraDevice>
#include <QAudioDevice>

#include "Managers/managercollection.h"
#include "Helpers/jsonhelper.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_logManager = ManagerCollection::AddManager<LogManager>();
    m_joystickManager = ManagerCollection::AddManager<JoystickManager>(this);
    m_keyboardManager = ManagerCollection::AddManager<KeyboardManager>();
    m_serialManager = ManagerCollection::AddManager<SerialManager>(this);
    m_vlcManager = ManagerCollection::AddManager<VlcManager>();
    m_programManager = ManagerCollection::AddManager<ProgramManager>(this);

    m_logManager->Initialize(ui);
    m_joystickManager->Initialize(ui);
    m_keyboardManager->Initialize(ui);
    m_serialManager->Initialize(ui);
    m_vlcManager->Initialize(ui);
    m_programManager->Initialize(ui);

    LoadSettings();

    this->installEventFilter(this);
    m_logManager->PrintLog("Global", "Initialization completed");
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

    if (!m_logManager->OnCloseEvent())
    {
        event->ignore();
        return;
    }

    SaveSettings();

    delete m_vlcManager;
    delete m_keyboardManager;
    delete m_logManager;

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
        if (widget->isActiveWindow() && !m_wasActivated)
        {
            m_logManager->raise();
            m_keyboardManager->raise();
            m_vlcManager->raise();
        }

        m_wasActivated = widget->isActiveWindow() && !widget->isMinimized();
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
