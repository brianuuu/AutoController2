#include "serialmanager.h"

#include "defines.h"

SerialManager::SerialManager
(
    QComboBox *list,
    QPushButton *btnRefresh,
    QPushButton *btnConnect
)
    : m_list(list)
    , m_btnRefresh(btnRefresh)
    , m_btnConnect(btnConnect)
{
    connect(m_btnRefresh, &QPushButton::clicked, this, &SerialManager::OnRefreshList);
    connect(m_btnConnect, &QPushButton::clicked, this, &SerialManager::OnConnectClicked);

    connect(&m_serialPort, &QSerialPort::readyRead, this, &SerialManager::OnReadyRead);
    connect(&m_serialPort, &QSerialPort::errorOccurred, this, &SerialManager::OnErrorOccured);

    OnRefreshList();
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
        // TODO: log
        m_list->setEnabled(false);
        m_btnRefresh->setEnabled(false);
        m_btnConnect->setEnabled(true);
        m_btnConnect->setText("Disconnect");

        // TODO: remove
        uint32_t m_buttonFlag = 1UL << 24 | 1UL << 26;

        QByteArray ba;
        ba.append((char)0xFF); // mode = FF
        ba.append((char)(m_buttonFlag & 0x000000FF));
        ba.append((char)((m_buttonFlag & 0x0000FF00) >> 8));
        ba.append((char)((m_buttonFlag & 0x00FF0000) >> 16));
        ba.append((char)((m_buttonFlag & 0xFF000000) >> 24));
        m_serialPort.write(ba);

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
        // TODO: log
    }

    m_list->setEnabled(true);
    m_btnRefresh->setEnabled(true);
    m_btnConnect->setEnabled(true);
    m_btnConnect->setText("Connect");

    m_serialState = SerialState::Disconnected;
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
        // TODO: use common function
        QByteArray ba;
        ba.append((char)0xFF);
        ba.append(4, (char)0);
        m_serialPort.write(ba);
    }
    else
    {
        m_serialState = SerialState::Disconnected;
        QMessageBox::critical(this, "Error", "Failed to connect serial port!", QMessageBox::Ok);
    }
}

void SerialManager::Disconnect()
{
    if (m_serialState == SerialState::Disconnecting)
    {
        return;
    }

    if (m_serialPort.isOpen())
    {
        // clear button
        QByteArray ba;
        ba.append((char)0);
        m_serialPort.write(ba);

        QTimer::singleShot(50, this, &SerialManager::OnDisconnectTimeout);
    }

    m_serialState = SerialState::Disconnecting;
    m_btnConnect->setEnabled(false);
    m_btnConnect->setText("Disconnecting...");
}
