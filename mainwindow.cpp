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

    m_serialManager = new SerialManager(
        this,
        ui->CB_SerialPort,
        ui->PB_SerialRefresh,
        ui->PB_SerialConnect
    );

    m_audioManager = new AudioManager(
        this,
        ui->CB_AudioInput,
        ui->CB_AudioOutput,
        ui->CB_AudioDisplay,
        ui->HS_Volume
    );

    m_videoManager = new VideoManager(
        this,
        ui->CB_CameraDevice,
        ui->CB_Resolution,
        ui->PB_CameraStart
    );
}

MainWindow::~MainWindow()
{
    delete m_videoManager;
    delete m_audioManager;
    delete m_serialManager;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!m_serialManager->OnCloseEvent())
    {
        event->ignore();
        return;
    }

    if (!m_videoManager->OnCloseEvent())
    {
        event->ignore();
        return;
    }

    QMainWindow::closeEvent(event);
}
