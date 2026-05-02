#include "serialholder.h"

#include "Managers/keyboardmanager.h"
#include "Managers/logmanager.h"
#include "Managers/profileManager.h"
#include "Types/buttontype.h"
#include "defines.h"

SerialHolder::SerialHolder(QObject *parent)
    : QThread{parent}
{
    m_profileManager = ManagerCollection::GetManager<ProfileManager>();

    connect(&m_serialPort, &QSerialPort::readyRead, this, &SerialHolder::OnReadyRead);
    connect(&m_serialPort, &QSerialPort::errorOccurred, this, &SerialHolder::OnErrorOccured);
    m_serialPort.moveToThread(this);

    LogManager* logManager = ManagerCollection::GetManager<LogManager>();
    connect(this, &SerialHolder::notifyLog, logManager, &LogManager::PrintLog);

    KeyboardManager* keyboardManager = ManagerCollection::GetManager<KeyboardManager>();
    connect(this, &SerialHolder::notifyDisplayButton, keyboardManager, &KeyboardManager::OnDisplayButton);

    m_commandTimer.setSingleShot(true);
    m_commandTimer.moveToThread(this);
    m_commandTimer.setTimerType(Qt::PreciseTimer);
    connect(&m_commandTimer, &QTimer::timeout, this, [this] { SendCurrentCommand(); });

    this->moveToThread(this);
    this->start();
}

SerialHolder::~SerialHolder()
{
    this->quit();
    this->wait();
}

bool SerialHolder::IsOpen() const
{
    QMutexLocker locker(&m_mutex);
    return m_serialPort.isOpen();
}

bool SerialHolder::IsConnected() const
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
    QMutexLocker locker(&m_mutex);
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
    QMutexLocker locker(&m_mutex);
    if (m_serialState == SerialState::FeedbackOK)
    {
        m_serialState = SerialState::Connected;
        emit notifyLog("Serial", "Serial Connected (Version = " + QString::number(m_serialVersion) + ")", LOG_Success);
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
        emit notifyLog("Serial", "Serial Disconnected", LOG_Warning);
    }

    m_serialState = SerialState::Disconnected;
    emit notifySerialStatus();
    emit notifyDisconnectTimeout();
}

void SerialHolder::OnSendCommand(const QString &command)
{
    m_command = command;
    m_commandIndex = 0;
    m_commandLoopCounts.clear();
    m_infiniteLoopCount = 0;

    SendCurrentCommand();
}

void SerialHolder::OnClearCommand()
{
    if (m_command.isEmpty()) return;

    m_command.clear();
    m_commandIndex = 0;
    m_commandTimer.stop();
    m_commandLoopCounts.clear();
    m_infiniteLoopCount = 0;

    SendButton(0);
    emit notifyDisplayButton(0);
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

void SerialHolder::SendCurrentCommand(bool isLoopCount)
{
    // check completion
    if (m_commandIndex == -1 || m_commandIndex >= m_command.size())
    {
        OnClearCommand();
        emit notifyCommandFinished();
        return;
    }

    qsizetype endIndex = m_command.indexOf(',', m_commandIndex + 1);
    QString str = m_command.mid(m_commandIndex, endIndex == -1 ? -1 : endIndex - m_commandIndex);

    // look for loop start
    qsizetype const loopStartIndex = str.indexOf('(');
    if (loopStartIndex == 0)
    {
        m_commandIndex++;
        m_commandLoopCounts.push_back(-1);
        SendCurrentCommand();
        return;
    }

    // look for loop end
    qsizetype const loopEndIndex = str.indexOf(')');
    if (loopEndIndex >= 0)
    {
        if (loopEndIndex == 0)
        {
            // first index is ')' expecting loop count next
            m_commandIndex++;
            SendCurrentCommand(true);
            return;
        }
        else
        {
            // remove all char after ')' so number remains
            str = str.mid(0, loopEndIndex);
            endIndex = m_commandIndex + loopEndIndex - 1;
        }
    }

    quint32 buttonFlag = 0;
    QPointF lStick(0,0);
    QPointF rStick(0,0);

    QStringList const buttons = str.split('|');
    for (int b = 0; b < buttons.size() - 1; b++)
    {
        QString const& button = buttons[b].toLower();
        if (button.startsWith("lx") || button.startsWith("ly") || button.startsWith("rx") || button.startsWith("ry"))
        {
            qreal const stickPos = button.mid(2).toDouble();
            if (button.startsWith("lx"))
            {
                lStick.setX(stickPos);
            }
            else if (button.startsWith("ly"))
            {
                lStick.setY(stickPos);
            }
            else if (button.startsWith("rx"))
            {
                rStick.setX(stickPos);
            }
            else if (button.startsWith("ry"))
            {
                rStick.setY(stickPos);
            }
        }
        else
        {
            buttonFlag |= StringToButtonFlag(button);
        }
    }

    int duration = buttons.back().toInt();
    if (isLoopCount)
    {
        // found a loop count
        int& loopLeft = m_commandLoopCounts.back();
        if (loopLeft == -1)
        {
            loopLeft = duration;
        }

        if (loopLeft == 1)
        {
            // immediately run next command if loop finished
            m_commandLoopCounts.pop_back();

            if (endIndex == -1)
            {
                m_commandIndex = -1;
            }
            else
            {
                m_commandIndex = endIndex + 1;
            }

            SendCurrentCommand();
            return;
        }
        else
        {
            m_commandIndex--;
            if (loopLeft > 1)
            {
                // if loopCount is 0 it loops forever
                loopLeft--;
            }

            if (m_debugLoop || m_profileManager->GetDebugButton())
            {
                if (loopLeft == 0)
                {
                    m_infiniteLoopCount++;
                    emit notifyLog("Serial", "Infinite Loop Count: " + QString::number(m_infiniteLoopCount));
                    emit notifyInfiniteLoop();
                }
                else
                {
                    emit notifyLog("Serial", "Loop Left: " + QString::number(loopLeft) + " (Nest " + QString::number(m_commandLoopCounts.size() - 1) + ")");
                }
            }

            // roll back to '('
            int loopEndCount = 0;
            while (m_commandIndex > 0)
            {
                m_commandIndex--;
                if (m_command[m_commandIndex] == ')')
                {
                    loopEndCount++;
                }
                else if (m_command[m_commandIndex] == '(')
                {
                    if (loopEndCount == 0)
                    {
                        m_commandIndex++;
                        SendCurrentCommand();
                        return;
                    }
                    else
                    {
                        loopEndCount--;
                    }
                }
            }
        }
    }
    else
    {
        if (m_profileManager->GetDebugButton())
        {
            emit notifyLog("Serial", "Button: \"" + str + "\"");
        }

        OnSendButton(buttonFlag, lStick, rStick);
        emit notifyDisplayButton(buttonFlag, lStick, rStick);
        m_commandTimer.start(duration);
    }

    if (endIndex == -1)
    {
        m_commandIndex = -1;
    }
    else
    {
        m_commandIndex = endIndex + 1;
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
