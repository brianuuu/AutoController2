#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <qmediadevices.h>
#include <QCameraDevice>
#include <QAudioDevice>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_logManager = new LogManager(
        ui->PB_OutputWindow
    );

    m_serialManager = new SerialManager(
        m_logManager,
        ui->CB_SerialPort,
        ui->PB_SerialRefresh,
        ui->PB_SerialConnect,
        this
    );

    m_vlcManager = new VlcManager(
        m_logManager,
        ui->CB_CameraDevice,
        ui->CB_Resolution,
        ui->CB_AudioInput,
        ui->CB_AudioOutput,
        ui->CB_AudioDisplay,
        ui->HS_Volume,
        ui->PB_CameraRefresh,
        ui->PB_CameraStart,
        ui->PB_Screenshot
    );

    m_logManager->PrintLog("System", "Initialization completed");
}

MainWindow::~MainWindow()
{
    delete m_vlcManager;
    delete m_logManager;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!m_serialManager->OnCloseEvent())
    {
        event->ignore();
        return;
    }

    if (!m_vlcManager->OnCloseEvent())
    {
        event->ignore();
        return;
    }

    if (!m_logManager->OnCloseEvent())
    {
        event->ignore();
        return;
    }

    QMainWindow::closeEvent(event);
}
