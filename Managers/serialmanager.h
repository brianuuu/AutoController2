#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QCloseEvent>
#include <QComboBox>
#include <QMessageBox>
#include <QPushButton>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

#include "Helpers/serialholder.h"
#include "Managers/managercollection.h"

namespace Ui { class MainWindow; }

class SerialManager : public QWidget
{
    Q_OBJECT

public:
    explicit SerialManager(QWidget* parent = nullptr);
    ~SerialManager();

    static QString GetTypeID() { return "Serial"; }
    void Initialize(Ui::MainWindow* ui);

    bool OnCloseEvent();
    bool IsConnected() { return m_serialHolder && m_serialHolder->IsConnected(); }
    SerialHolder* GetHolder() { return m_serialHolder; }

    static bool VerifyCommand(QString const& command, QString& errorMsg);

signals:
    void notifyClose();
    void notifySerialConnect(QString const& name);
    void notifySerialDisconnect();

private slots:
    // UI
    void OnRefreshList();

    // serial
    void OnErrorOccured();
    void OnConnectClicked();
    void OnConnecting(bool failed);
    void OnConnectTimeout(bool failed, quint8 version = 0);
    void OnDisconnecting();
    void OnDisconnectTimeout();

private:
    void LoadSettings();
    void SaveSettings() const;

private:
    bool m_aboutToClose = false;

    // Managers
    KeyboardManager*m_keyboardManager = Q_NULLPTR;
    LogManager*     m_logManager = Q_NULLPTR;

    // UI
    QComboBox*      m_list = Q_NULLPTR;
    QPushButton*    m_btnRefresh = Q_NULLPTR;
    QPushButton*    m_btnConnect = Q_NULLPTR;

    // Serial
    SerialHolder*   m_serialHolder = Q_NULLPTR;
};

#endif // SERIALMANAGER_H
