#include "serialmanager.h"

#include "defines.h"
#include "Enums/system.h"
#include "Helpers/jsonhelper.h"
#include "Managers/keyboardmanager.h"
#include "Managers/logmanager.h"

SerialManager::SerialManager(QWidget* parent) : QWidget(parent)
{
    connect(this, &SerialManager::notifyClose, parent, &QWidget::close);

    m_serialHolder = new SerialHolder();
    m_serialHolder->moveToThread(m_serialHolder);
    m_serialHolder->start();
}

SerialManager::~SerialManager()
{
    delete m_serialHolder;
}

void SerialManager::Initialize(Ui::MainWindow *ui)
{
    m_keyboardManager = ManagerCollection::GetManager<KeyboardManager>();
    m_logManager = ManagerCollection::GetManager<LogManager>();

    m_list = ui->CB_SerialPort;
    m_btnRefresh = ui->PB_SerialRefresh;
    m_btnConnect = ui->PB_SerialConnect;

    connect(m_btnRefresh, &QPushButton::clicked, this, &SerialManager::OnRefreshList);
    connect(m_btnConnect, &QPushButton::clicked, this, &SerialManager::OnConnectClicked);

    connect(this, &SerialManager::notifySerialConnect, m_serialHolder, &SerialHolder::OnConnectClicked);
    connect(this, &SerialManager::notifySerialDisconnect, m_serialHolder, &SerialHolder::OnDisconnectClicked);
    connect(m_serialHolder, &SerialHolder::notifyErrorOccured, this, &SerialManager::OnErrorOccured);
    connect(m_serialHolder, &SerialHolder::notifyConnecting, this, &SerialManager::OnConnecting);
    connect(m_serialHolder, &SerialHolder::notifyConnectTimeout, this, &SerialManager::OnConnectTimeout);
    connect(m_serialHolder, &SerialHolder::notifyDisconnecting, this, &SerialManager::OnDisconnecting);
    connect(m_serialHolder, &SerialHolder::notifyDisconnectTimeout, this, &SerialManager::OnDisconnectTimeout);

    OnRefreshList();
    LoadSettings();
}

bool SerialManager::OnCloseEvent()
{
    // main window is closing
    if (m_serialHolder->IsOpen())
    {
        m_aboutToClose = true;
        emit notifySerialDisconnect();
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

    if (command.contains("))"))
    {
        errorMsg = "Double loop ending '))' is not allowed";
        return false;
    }

    bool isLoopCount = false;
    bool hasInfiniteLoop = false;
    for (int i = 0; i < command.size();)
    {
        qsizetype endIndex = command.indexOf(',', i + 1);
        QString str = command.mid(i, endIndex == -1 ? -1 : endIndex - i);

        if (hasInfiniteLoop)
        {
            errorMsg = "No more command should be after infinite loop";
            return false;
        }

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

        if (isLoopCount && duration == 0)
        {
            hasInfiniteLoop = true;
        }
        isLoopCount = false;

        if (endIndex == -1)
        {
            break;
        }
        else
        {
            i = endIndex + 1;
        }
    }

    if (command.endsWith(',') || command.endsWith('|') || command.endsWith('-') || command.endsWith('.') || command.endsWith(')'))
    {
        errorMsg = "Ending character not expected";
        return false;
    }

    return true;
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

void SerialManager::OnErrorOccured()
{
    OnDisconnectTimeout();
    OnRefreshList();
    QMessageBox::critical(this, "Error", "Serial port disconnected unexpectedly!", QMessageBox::Ok);
}

void SerialManager::OnConnectClicked()
{
    emit notifySerialConnect(m_list->currentData().toString());
}

void SerialManager::OnConnecting(bool failed)
{
    if (failed)
    {
        QMessageBox::critical(this, "Error", "Failed to connect serial port!", QMessageBox::Ok);
    }
    else
    {
        m_list->setEnabled(false);
        m_btnRefresh->setEnabled(false);
        m_btnConnect->setEnabled(false);
        m_btnConnect->setText("Connecting...");
    }
}

void SerialManager::OnConnectTimeout(bool failed, quint8 version)
{
    if (failed)
    {
        QString msg;
        if (version > 0)
        {
            msg = "AutoController2.hex version is not matching, please install the newest version.";
            msg += "\nVersion Detected: " + QString::number(version);
            msg += "\nCurrent Version: " + QString::number(SERIAL_VERSION);
        }
        else
        {
            msg = "Failed to receive feedback from Arduino/Teensy.";
        }
        QMessageBox::critical(this, "Error", msg, QMessageBox::Ok);
    }
    else
    {
        m_list->setEnabled(false);
        m_btnRefresh->setEnabled(false);
        m_btnConnect->setEnabled(true);
        m_btnConnect->setText("Disconnect");
    }
}

void SerialManager::OnDisconnecting()
{
    m_btnConnect->setEnabled(false);
    m_btnConnect->setText("Disconnecting...");
}

void SerialManager::OnDisconnectTimeout()
{
    m_list->setEnabled(true);
    m_btnRefresh->setEnabled(true);
    m_btnConnect->setEnabled(true);
    m_btnConnect->setText("Connect");

    if (m_aboutToClose)
    {
        m_aboutToClose = false;
        emit notifyClose();
    }
}
