#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QCloseEvent>
#include <QComboBox>
#include <QMessageBox>
#include <QPushButton>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

#include "../ui_mainwindow.h"
#include "logmanager.h"

class SerialManager : public QWidget
{
    Q_OBJECT

public:
    explicit SerialManager(QWidget* parent = nullptr);
    static QString GetTypeID() { return "Serial"; }
    void Initialize(Ui::MainWindow* ui);

    bool OnCloseEvent();
    bool IsConnected() const { return m_serialState == SerialState::Connected; }

    // command
    void SendButton(quint32 buttonFlag, quint8 lx = 128, quint8 ly = 128, quint8 rx = 128, quint8 ry = 128);
    bool VerifyCommand(QString const& command, QString& errorMsg);
    bool SendCommand(QString const& command);
    void ClearCommand();

private: // types
    enum class SerialState
    {
        Disconnected,
        FeedbackTest,
        FeedbackOK,
        FeedbackFailed,
        Disconnecting,
        Connected,
    };

signals:
    void notifyClose();
    void notifySerialStatus();

private slots:
    // UI
    void OnRefreshList();

    // serial
    void OnReadyRead();
    void OnErrorOccured(QSerialPort::SerialPortError error);
    void OnConnectClicked();
    void OnConnectTimeout();
    void OnDisconnectTimeout();

    // command
    void OnSendCurrentCommand(bool isLoopCount = false);

private:
    void LoadSettings();
    void SaveSettings() const;

    // serial
    void Connect(QString const& port);
    void Disconnect();

private:
    bool m_aboutToClose = false;

    // UI
    LogManager*     m_logManager = Q_NULLPTR;
    QComboBox*      m_list = Q_NULLPTR;
    QPushButton*    m_btnRefresh = Q_NULLPTR;
    QPushButton*    m_btnConnect = Q_NULLPTR;

    // serial
    QSerialPort     m_serialPort;
    SerialState     m_serialState = SerialState::Disconnected;
    quint8          m_serialVersion = 0;

    // command
    QString         m_command;
    int             m_commandIndex = 0;
    QVector<int>    m_commandLoopCounts;
    QTimer          m_commandTimer;
};

#endif // SERIALMANAGER_H
