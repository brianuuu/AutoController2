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

    m_videoManager = new VideoManager(
        this,
        ui->CB_CameraDevice,
        ui->CB_Resolution,
        ui->PB_CameraStart
    );

    const QList<QAudioDevice> audioDevices = QMediaDevices::audioInputs();
    for (const QAudioDevice &device : audioDevices)
    {
        qDebug() << "Description: " << device.description();
    }

    const QList<QAudioDevice> audioOutputs = QMediaDevices::audioOutputs();
    for (const QAudioDevice &device : audioOutputs)
    {
        qDebug() << "Description: " << device.description();
    }
}

MainWindow::~MainWindow()
{
    delete m_serialManager;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!m_serialManager->OnCloseEvent(event))
    {
        return;
    }
}
