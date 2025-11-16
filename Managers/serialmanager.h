#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QComboBox>
#include <QPushButton>
#include <QSerialPortInfo>

class SerialManager : QObject
{
    Q_OBJECT

public:
    SerialManager
    (
        QComboBox* list,
        QPushButton* btnRefresh,
        QPushButton* btnConnect
    );

private:
    void RefreshList();

private:
    QComboBox* m_list = Q_NULLPTR;
    QPushButton* m_btnRefresh = Q_NULLPTR;
    QPushButton* m_btnConnect = Q_NULLPTR;
};

#endif // SERIALMANAGER_H
