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
    void OnRefreshList();

    // serial
    void OnReadyRead();
    void OnErrorOccured(QSerialPort::SerialPortError error);
    void OnConnectClicked();
    void OnConnectTimeout();
    void OnDisconnectTimeout();

private:
    // serial
    void Connect(QString const& port);
    void Disconnect();

    // command
    void SendButton(quint32 buttonFlag);

private:
    bool m_aboutToClose = false;

    QComboBox* m_list = Q_NULLPTR;
    QPushButton* m_btnRefresh = Q_NULLPTR;
    QPushButton* m_btnConnect = Q_NULLPTR;

    QSerialPort m_serialPort;
    SerialState m_serialState = SerialState::Disconnected;
    quint8 m_serialVersion = 0;

    QString m_command;
};

#endif // SERIALMANAGER_H
