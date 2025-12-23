#include "serialholder.h"

#include "Managers/managercollection.h"
#include "Managers/logmanager.h"
#include "defines.h"

SerialHolder::SerialHolder(QObject *parent)
    : QThread{parent}
{
    connect(&m_serialPort, &QSerialPort::readyRead, this, &SerialHolder::OnReadyRead);
    connect(&m_serialPort, &QSerialPort::errorOccurred, this, &SerialHolder::OnErrorOccured);
    m_serialPort.moveToThread(this);

    LogManager* logManager = ManagerCollection::GetManager<LogManager>();
    connect(this, &SerialHolder::notifyLog, logManager, &LogManager::PrintLog);
}

SerialHolder::~SerialHolder()
{
    this->quit();
    this->wait();
}

bool SerialHolder::IsOpen()
{
    QMutexLocker locker(&m_mutex);
    return m_serialPort.isOpen();
}

bool SerialHolder::IsConnected()
{
    QMutexLocker locker(&m_mutex);
    return m_serialState == SerialState::Connected;
}

void SerialHolder::OnReadyRead()
{
    QMutexLocker locker(&m_mutex);
    QByteArray ba = m_serialPort.readAll();
    if (m_serialState != SerialState::Connected)
    {
        if (!ba.isEmpty())
        {
            // Version checking
            m_serialVersion = ba.front();
            if (m_serialVersion == SERIAL_VERSION)
            {
                m_serialState = SerialState::FeedbackOK;
            }
            else
            {
                m_serialState = SerialState::FeedbackFailed;
            }
        }
        return;
    }
}

void SerialHolder::OnErrorOccured(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError)
    {
        OnDisconnectTimeout();
        emit notifyErrorOccured();
    }
}

void SerialHolder::OnConnectClicked(QString const& name)
{
    QMutexLocker locker(&m_mutex);
    if (m_serialPort.isOpen())
    {
        Disconnect();
    }
    else if (!name.isEmpty())
    {
        Connect(name);
    }
}

void SerialHolder::OnConnectTimeout()
{
    if (m_serialState == SerialState::FeedbackOK)
    {
        m_serialState = SerialState::Connected;
        emit notifyLog("Global", "Serial Connected", LOG_Success);
        emit notifySerialStatus();
        emit notifyConnectTimeout(false);
        return;
    }

    OnDisconnectTimeout();
    emit notifyConnectTimeout(true, m_serialVersion);
}

void SerialHolder::OnDisconnectClicked()
{
    QMutexLocker locker(&m_mutex);
    Disconnect();
}

void SerialHolder::OnDisconnectTimeout()
{
    QMutexLocker locker(&m_mutex);
    if (m_serialPort.isOpen())
    {
        m_serialPort.close();
        emit notifyLog("Global", "Serial Disconnected", LOG_Warning);
    }

    m_serialState = SerialState::Disconnected;
    emit notifySerialStatus();
    emit notifyDisconnectTimeout();
}

void SerialHolder::Connect(const QString &name)
{
    QMutexLocker locker(&m_mutex);
    m_serialPort.setPortName(name);
    m_serialPort.setBaudRate(QSerialPort::Baud9600);
    m_serialPort.setDataBits(QSerialPort::Data8);
    m_serialPort.setParity(QSerialPort::NoParity);
    m_serialPort.setStopBits(QSerialPort::OneStop);
    m_serialPort.setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialPort.open(QIODevice::ReadWrite))
    {
        m_serialState = SerialState::FeedbackTest;
        QTimer::singleShot(500, this, &SerialHolder::OnConnectTimeout);

        // Send a nothing command and check if it returns a feedback
        SendButton(0);

        emit notifyConnecting(false);
    }
    else
    {
        m_serialState = SerialState::Disconnected;
        emit notifyConnecting(true);
    }
}

void SerialHolder::Disconnect()
{
    QMutexLocker locker(&m_mutex);
    if (m_serialState == SerialState::Disconnecting) return;

    if (m_serialPort.isOpen())
    {
        // clear button, we don't want feedback
        QByteArray ba;
        ba.append((char)0);
        m_serialPort.write(ba);

        QTimer::singleShot(50, this, &SerialHolder::OnDisconnectTimeout);

        m_serialState = SerialState::Disconnecting;
        emit notifySerialStatus();
        emit notifyDisconnecting();
    }
    else
    {
        OnDisconnectTimeout();
    }
}

void SerialHolder::OnSendButton(quint32 buttonFlag, QPointF lStick, QPointF rStick)
{
    QMutexLocker locker(&m_mutex);
    if (!m_serialPort.isOpen()) return;

    quint8 lx = qCeil((lStick.x() + 1.0) * 0.5 * 255);
    quint8 ly = qCeil((-lStick.y() + 1.0) * 0.5 * 255);
    quint8 rx = qCeil((rStick.x() + 1.0) * 0.5 * 255);
    quint8 ry = qCeil((-rStick.y() + 1.0) * 0.5 * 255);

    SendButton(buttonFlag, lx, ly, rx, ry);
}

void SerialHolder::SendButton(quint32 buttonFlag, quint8 lx, quint8 ly, quint8 rx, quint8 ry)
{
    QMutexLocker locker(&m_mutex);
    if (!m_serialPort.isOpen()) return;

    QByteArray ba;
    ba.append((char)0xFF); // mode = FF

    ba.append((char)(buttonFlag & 0x000000FF));
    ba.append((char)((buttonFlag & 0x0000FF00) >> 8));
    ba.append((char)((buttonFlag & 0x00FF0000) >> 16));
    ba.append((char)((buttonFlag & 0xFF000000) >> 24));

    ba.append((char)lx);
    ba.append((char)ly);
    ba.append((char)rx);
    ba.append((char)ry);

    m_serialPort.write(ba);
}
