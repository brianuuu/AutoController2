#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

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
        QComboBox* list,
        QPushButton* btnRefresh,
        QPushButton* btnConnect
    );

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

private slots:
    void OnRefreshList();
    void OnReadyRead();
    void OnErrorOccured(QSerialPort::SerialPortError error);
    void OnConnectClicked();
    void OnConnectTimeout();
    void OnDisconnectTimeout();

private:
    void Connect(QString const& port);
    void Disconnect();

private:
    QComboBox* m_list = Q_NULLPTR;
    QPushButton* m_btnRefresh = Q_NULLPTR;
    QPushButton* m_btnConnect = Q_NULLPTR;

    QSerialPort m_serialPort;
    SerialState m_serialState = SerialState::Disconnected;
    quint8 m_serialVersion = 0;
};

#endif // SERIALMANAGER_H
