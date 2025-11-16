#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_serialManager = new SerialManager(
        ui->CB_SerialPort,
        ui->PB_SerialRefresh,
        ui->PB_SerialConnect
    );
}

MainWindow::~MainWindow()
{
    delete m_serialManager;
    delete ui;
}
