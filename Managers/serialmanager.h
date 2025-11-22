#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QCloseEvent>
#include <QComboBox>
#include <QMessageBox>
#include <QPushButton>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>


class SerialManager : public QWidget
{
    Q_OBJECT

public:
    SerialManager
    (
        QWidget* parent,
        QComboBox* list,
        QPushButton* btnRefresh,
        QPushButton* btnConnect
    );

    bool OnCloseEvent(QCloseEvent *event);

    // command
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
    void SignalClose();

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
    // serial
    void Connect(QString const& port);
    void Disconnect();

    // command
    void SendButton(quint32 buttonFlag);

private:
    bool m_aboutToClose = false;

    // UI
    QComboBox* m_list = Q_NULLPTR;
    QPushButton* m_btnRefresh = Q_NULLPTR;
    QPushButton* m_btnConnect = Q_NULLPTR;

    // serial
    QSerialPort m_serialPort;
    SerialState m_serialState = SerialState::Disconnected;
    quint8 m_serialVersion = 0;

    // command
    QString m_command;
    int m_commandIndex = 0;
    QVector<int> m_commandLoopCounts;
    QTimer m_commandTimer;
};

#endif // SERIALMANAGER_H
