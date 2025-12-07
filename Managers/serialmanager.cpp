#include "serialmanager.h"

#include "managercollection.h"
#include "keyboardmanager.h"
#include "defines.h"
#include "Enums/system.h"
#include "Helpers/jsonhelper.h"

SerialManager::SerialManager(QWidget* parent) : QWidget(parent)
{
    connect(this, &SerialManager::notifyClose, parent, &QWidget::close);
}

void SerialManager::Initialize(Ui::MainWindow *ui)
{
    m_logManager = ManagerCollection::GetManager<LogManager>();
    m_list = ui->CB_SerialPort;
    m_btnRefresh = ui->PB_SerialRefresh;
    m_btnConnect = ui->PB_SerialConnect;

    connect(m_btnRefresh, &QPushButton::clicked, this, &SerialManager::OnRefreshList);
    connect(m_btnConnect, &QPushButton::clicked, this, &SerialManager::OnConnectClicked);

    connect(&m_serialPort, &QSerialPort::readyRead, this, &SerialManager::OnReadyRead);
    connect(&m_serialPort, &QSerialPort::errorOccurred, this, &SerialManager::OnErrorOccured);

    connect(&m_commandTimer, &QTimer::timeout, this, [this]{ OnSendCurrentCommand(); } );

    KeyboardManager* keyboardManager = ManagerCollection::GetManager<KeyboardManager>();
    connect(this, &SerialManager::notifySerialStatus, keyboardManager, &KeyboardManager::OnUpdateStatus);

    OnRefreshList();
    LoadSettings();
}

bool SerialManager::OnCloseEvent()
{
    // main window is closing
    if (m_serialPort.isOpen())
    {
        m_aboutToClose = true;
        Disconnect();
        return false;
    }

    SaveSettings();
    return true;
}

void SerialManager::LoadSettings()
{
    QJsonObject settings = JsonHelper::ReadSetting("SerialSettings");
    {
        QVariant portName;
        if (JsonHelper::ReadValue(settings, "PortName", portName) && !portName.toString().isEmpty())
        {
            m_list->setCurrentText(portName.toString());
        }
    }
}

void SerialManager::SaveSettings() const
{
    QJsonObject settings;
    settings.insert("PortName", m_list->currentText());

    JsonHelper::WriteSetting("SerialSettings", settings);
}

bool SerialManager::VerifyCommand(const QString &command, QString &errorMsg)
{
    if (command.isEmpty())
    {
        errorMsg = "Command is empty";
        return false;
    }

    if (command.count('(') != command.count(')'))
    {
        errorMsg = "Number of '(' is not matching number of ')'";
        return false;
    }

    bool isLoopCount = false;
    for (int i = 0; i < command.size();)
    {
        qsizetype endIndex = command.indexOf(',', i + 1);
        QString str = command.mid(i, endIndex == -1 ? -1 : endIndex - i);

        // look for loop start
        qsizetype const loopStartIndex = str.indexOf('(');
        if (loopStartIndex > 0)
        {
            errorMsg = "Loop start '(' is not expected at index " + QString::number(i + loopStartIndex);
            return false;
        }
        else if (loopStartIndex == 0)
        {
            i++;
            continue;
        }

        // look for loop end
        qsizetype const loopEndIndex = str.indexOf(')');
        if (loopEndIndex >= 0)
        {
            if (loopEndIndex == 0)
            {
                // first index is ')' expecting loop count next
                i++;
                isLoopCount = true;
                continue;
            }
            else
            {
                // remove all char after ')' so number remains
                str = str.mid(0, loopEndIndex);
                endIndex = i + loopEndIndex - 1;
            }
        }

        QStringList const buttons = str.split('|');
        if (buttons.empty() || (buttons.size() == 1 && !isLoopCount))
        {
            errorMsg = "Command '" + str + "' is invalid at index " + QString::number(i) + ", expecting Button1|Button2|...|Duration";
            return false;
        }

        for (int b = 0; b < buttons.size() - 1; b++)
        {
            QString const& button = buttons[b].toLower();
            if (button.startsWith("lx") || button.startsWith("ly") || button.startsWith("rx") || button.startsWith("ry"))
            {
                bool ok = false;
                qreal const stickPos = button.mid(2).toDouble(&ok);
                if (!ok || qAbs(stickPos) > 1.0)
                {
                    errorMsg = "'" + button + "' does not have valid stick position (between -1.0 to 1.0) at index " + QString::number(i);
                    return false;
                }
            }
            else if (StringToButton(button) == BTN_COUNT)
            {
                errorMsg = "'" + button + "' is not a recognized button at index " + QString::number(i);
                return false;
            }
        }

        bool ok = false;
        int duration = buttons.back().toInt(&ok);
        if (!ok || (isLoopCount && duration < 0) || (!isLoopCount && duration <= 0))
        {
            errorMsg = QString(isLoopCount ? "Loop Count" : "Duration") + " '" + buttons.back() + "' is invalid at index " + QString::number(i);
            return false;
        }

        if (endIndex == -1)
        {
            break;
        }
        else
        {
            i = endIndex + 1;
        }
    }

    if (command.endsWith(',') || command.endsWith('|') || command.endsWith('-') || command.endsWith('.'))
    {
        errorMsg = "Ending character not expected";
        return false;
    }

    return true;
}

bool SerialManager::SendCommand(const QString &command)
{
    ClearCommand();

    if (!m_serialPort.isOpen()) return false;

    QString errorMsg;
    if (!VerifyCommand(command, errorMsg))
    {
        m_logManager->PrintLog("Global", errorMsg, LOG_Error);
        return false;
    }

    m_command = command;
    OnSendCurrentCommand();
    return true;
}

void SerialManager::ClearCommand()
{
    m_command.clear();
    m_commandIndex = 0;
    m_commandLoopCounts.clear();
    m_commandTimer.stop();
}

//-----------------------------------------------------------
// Slots
//-----------------------------------------------------------
void SerialManager::OnRefreshList()
{
    m_list->clear();

    for (QSerialPortInfo const& info : QSerialPortInfo::availablePorts())
    {
        m_list->addItem(info.portName() + ": " + info.description(), info.portName());
    }

    m_btnConnect->setEnabled(m_list->count() > 0);
}

void SerialManager::OnReadyRead()
{
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

void SerialManager::OnErrorOccured(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError)
    {
        OnDisconnectTimeout();
        OnRefreshList();
        QMessageBox::critical(this, "Error", "Serial port disconnected unexpectedly!", QMessageBox::Ok);
    }
}

void SerialManager::OnConnectClicked()
{
    if (m_serialPort.isOpen())
    {
        Disconnect();
    }
    else if (m_list->currentIndex() != -1)
    {
        Connect(m_list->currentData().toString());
    }
}

void SerialManager::OnConnectTimeout()
{
    if (m_serialState == SerialState::FeedbackOK)
    {
        m_serialState = SerialState::Connected;
        m_logManager->PrintLog("Global", "Serial Connected", LOG_Success);
        emit notifySerialStatus();

        m_list->setEnabled(false);
        m_btnRefresh->setEnabled(false);
        m_btnConnect->setEnabled(true);
        m_btnConnect->setText("Disconnect");
        return;
    }

    OnDisconnectTimeout();

    QString msg;
    if (m_serialState == SerialState::FeedbackFailed && m_serialVersion > 0)
    {
        msg = "AutoController2.hex version is not matching, please install the newest version.";
        msg += "\nVersion Detected: " + QString::number(m_serialVersion);
        msg += "\nCurrent Version: " + QString::number(SERIAL_VERSION);
    }
    else
    {
        msg = "Failed to receive feedback from Arduino/Teensy.";
    }
    QMessageBox::critical(this, "Error", msg, QMessageBox::Ok);
}

void SerialManager::OnDisconnectTimeout()
{
    if (m_serialPort.isOpen())
    {
        m_serialPort.close();
        m_logManager->PrintLog("Global", "Serial Disconnected", LOG_Warning);
    }

    m_serialState = SerialState::Disconnected;
    emit notifySerialStatus();

    m_list->setEnabled(true);
    m_btnRefresh->setEnabled(true);
    m_btnConnect->setEnabled(true);
    m_btnConnect->setText("Connect");

    ClearCommand();

    if (m_aboutToClose)
    {
        m_aboutToClose = false;
        emit notifyClose();
    }
}

void SerialManager::OnSendCurrentCommand(bool isLoopCount)
{
    if (!m_serialPort.isOpen()) return;

    if (m_commandIndex == -1 || m_commandIndex >= m_command.size())
    {
        // finished
        SendButton(0);
        ClearCommand();
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
        OnSendCurrentCommand();
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
            OnSendCurrentCommand(true);
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
    quint8 lx = 128;
    quint8 ly = 128;
    quint8 rx = 128;
    quint8 ry = 128;

    QStringList const buttons = str.split('|');
    for (int b = 0; b < buttons.size() - 1; b++)
    {
        QString const& button = buttons[b].toLower();
        if (button.startsWith("lx") || button.startsWith("ly") || button.startsWith("rx") || button.startsWith("ry"))
        {
            qreal const stickPos = button.mid(2).toDouble();
            quint8 const actualPos = (quint8)((stickPos + 1.0) * 0.5 * 255);
            if (button.startsWith("lx")) lx = actualPos;
            if (button.startsWith("ly")) ly = 255 - actualPos;
            if (button.startsWith("rx")) rx = actualPos;
            if (button.startsWith("ry")) ry = 255 - actualPos;
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
            m_commandLoopCounts.pop_back();
        }
        else
        {
            m_commandIndex--;
            if (loopLeft > 1)
            {
                // if loopCount is 0 it loops forever
                loopLeft--;
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
                        OnSendCurrentCommand();
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
        //m_logManager->PrintLog("Globel", "Button: \"" + str + "\"");
        SendButton(buttonFlag, lx, ly, rx, ry);
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

//-----------------------------------------------------------
// Private methods
//-----------------------------------------------------------
void SerialManager::Connect(const QString &port)
{
    m_serialPort.setPortName(port);
    m_serialPort.setBaudRate(QSerialPort::Baud9600);
    m_serialPort.setDataBits(QSerialPort::Data8);
    m_serialPort.setParity(QSerialPort::NoParity);
    m_serialPort.setStopBits(QSerialPort::OneStop);
    m_serialPort.setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialPort.open(QIODevice::ReadWrite))
    {
        m_serialState = SerialState::FeedbackTest;

        m_list->setEnabled(false);
        m_btnRefresh->setEnabled(false);
        m_btnConnect->setEnabled(false);
        m_btnConnect->setText("Connecting...");

        QTimer::singleShot(500, this, &SerialManager::OnConnectTimeout);

        // Send a nothing command and check if it returns a feedback
        SendButton(0);
    }
    else
    {
        m_serialState = SerialState::Disconnected;
        QMessageBox::critical(this, "Error", "Failed to connect serial port!", QMessageBox::Ok);
    }
}

void SerialManager::Disconnect()
{
    if (m_serialState == SerialState::Disconnecting) return;

    if (m_serialPort.isOpen())
    {
        // clear button, we don't want feedback
        QByteArray ba;
        ba.append((char)0);
        m_serialPort.write(ba);

        QTimer::singleShot(50, this, &SerialManager::OnDisconnectTimeout);

        m_serialState = SerialState::Disconnecting;
        emit notifySerialStatus();

        m_btnConnect->setEnabled(false);
        m_btnConnect->setText("Disconnecting...");

        ClearCommand();
    }
    else
    {
        OnDisconnectTimeout();
    }
}

void SerialManager::SendButton(quint32 buttonFlag, quint8 lx, quint8 ly, quint8 rx, quint8 ry)
{
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
