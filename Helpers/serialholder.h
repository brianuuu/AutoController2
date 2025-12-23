#ifndef SERIALHOLDER_H
#define SERIALHOLDER_H

#include <QMutex>
#include <QPointF>
#include <QSerialPort>
#include <QThread>
#include <QTimer>

#include "Enums/system.h"

enum class SerialState
{
    Disconnected,
    FeedbackTest,
    FeedbackOK,
    FeedbackFailed,
    Disconnecting,
    Connected,
};

class SerialHolder : public QThread
{
    Q_OBJECT

public:
    explicit SerialHolder(QObject *parent = nullptr);
    ~SerialHolder();

    bool IsOpen();
    bool IsConnected();

signals:
    void notifyErrorOccured();
    void notifySerialStatus();
     void notifyLog(QString const& category, QString const& log, LogType type = LOG_Normal) const;

    // signal to update UI
    void notifyConnecting(bool failed);
    void notifyConnectTimeout(bool failed, quint8 version = 0);
    void notifyDisconnecting();
    void notifyDisconnectTimeout();

public slots:
    // serial
    void OnReadyRead();
    void OnErrorOccured(QSerialPort::SerialPortError error);
    void OnConnectClicked(QString const& name);
    void OnConnectTimeout();
    void OnDisconnectClicked();
    void OnDisconnectTimeout();
    void OnSendButton(quint32 buttonFlag, QPointF lStick = QPointF(), QPointF rStick = QPointF());

private:
    void Connect(QString const& name);
    void Disconnect();

    void SendButton(quint32 buttonFlag, quint8 lx = 128, quint8 ly = 128, quint8 rx = 128, quint8 ry = 128);

private:
    QRecursiveMutex m_mutex;
    QSerialPort     m_serialPort;
    SerialState     m_serialState = SerialState::Disconnected;
    quint8          m_serialVersion = 0;
};

#endif // SERIALHOLDER_H
