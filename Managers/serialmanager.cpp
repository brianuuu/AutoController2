#include "serialmanager.h"

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
    connect(m_btnRefresh, &QPushButton::clicked, this, [this]{ RefreshList(); });
    connect(m_btnConnect, &QPushButton::clicked, this, [this]{ ; });

    RefreshList();
}

void SerialManager::RefreshList()
{
    m_list->clear();

    for (QSerialPortInfo const& info : QSerialPortInfo::availablePorts())
    {
        m_list->addItem(info.portName() + ": " + info.description(), info.portName());
    }

    m_btnConnect->setEnabled(m_list->count() > 0);
}
