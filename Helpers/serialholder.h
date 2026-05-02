#ifndef SERIALHOLDER_H
#define SERIALHOLDER_H

#include <QMutex>
#include <QPointF>
#include <QSerialPort>
#include <QThread>
#include <QTimer>

#include "Managers/managercollection.h"
#include "Types/logtype.h"

class SerialHolder : public QThread
{
    Q_OBJECT

public:
    explicit SerialHolder(QObject *parent = nullptr);
    ~SerialHolder();

    bool IsOpen() const;
    bool IsConnected() const;

    void SetDebugLoop(bool enable) { m_debugLoop = enable; }

signals:
    void notifyErrorOccured();
    void notifySerialStatus();
    void notifyLog(QString const& category, QString const& log, LogType type = LOG_Normal) const;

    // signals to update UI
    void notifyConnecting(bool failed);
    void notifyConnectTimeout(bool failed, quint8 version = 0);
    void notifyDisconnecting();
    void notifyDisconnectTimeout();

    // run command
    void notifyCommandFinished();
    void notifyDisplayButton(quint32 buttonFlag, QPointF lStick = QPointF(), QPointF rStick = QPointF());
    void notifyInfiniteLoop();

public slots:
    // serial
    void OnReadyRead();
    void OnErrorOccured(QSerialPort::SerialPortError error);
    void OnConnectClicked(QString const& name);
    void OnConnectTimeout();
    void OnDisconnectClicked();
    void OnDisconnectTimeout();

    void OnSendCommand(QString const& command);
    void OnClearCommand();

    void OnSendButton(quint32 buttonFlag, QPointF lStick = QPointF(), QPointF rStick = QPointF());

private:
    void Connect(QString const& name);
    void Disconnect();

    void SendCurrentCommand(bool isLoopCount = false);
    void SendButton(quint32 buttonFlag, quint8 lx = 128, quint8 ly = 128, quint8 rx = 128, quint8 ry = 128);

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

private:
    ProfileManager* m_profileManager = Q_NULLPTR;

    mutable QRecursiveMutex m_mutex;
    QSerialPort     m_serialPort;
    SerialState     m_serialState = SerialState::Disconnected;
    quint8          m_serialVersion = 0;

    QString         m_command;
    int             m_commandIndex = 0;
    QTimer          m_commandTimer;
    QVector<int>    m_commandLoopCounts;

    int             m_infiniteLoopCount = 0;
    bool            m_debugLoop = false;
};

#endif // SERIALHOLDER_H
