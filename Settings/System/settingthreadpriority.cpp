#include "settingthreadpriority.h"

namespace Setting::System {

SettingThreadPriority::SettingThreadPriority(const QString &name, QThread::Priority defaultPriority)
    : SettingComboBox(name, GetAvailableList())
{
    m_defaultPriority = defaultPriority;
    setCurrentText(PriorityToString(m_defaultPriority));
}

void SettingThreadPriority::ResetDefault()
{
    setCurrentText(PriorityToString(m_defaultPriority));
}

QString SettingThreadPriority::PriorityToString(QThread::Priority priority)
{
    switch (priority)
    {
    case QThread::IdlePriority:         return "Idle";
    case QThread::LowestPriority:       return "Lowest";
    case QThread::LowPriority:          return "Low";
    case QThread::NormalPriority:       return "Normal";
    case QThread::HighPriority:         return "High";
    case QThread::HighestPriority:      return "Highest";
    case QThread::TimeCriticalPriority: return "Realtime";
    case QThread::InheritPriority:      return "Inherited";
        break;
    }
    return "Unknown";
}

QThread::Priority SettingThreadPriority::StringToPriority(const QString &str)
{
    for (int i = QThread::IdlePriority; i <= QThread::InheritPriority; i++)
    {
        QThread::Priority const priority = QThread::Priority(i);
        if (str == PriorityToString(priority))
        {
            return priority;
        }
    }

    return QThread::InheritPriority;
}

QStringList SettingThreadPriority::GetAvailableList()
{
    QStringList list;
    for (int i = QThread::TimeCriticalPriority; i >= QThread::LowestPriority; i--)
    {
        QThread::Priority const priority = QThread::Priority(i);
        list << PriorityToString(priority);
    }

    return list;
}

QThread::Priority SettingThreadPriority::GetPriority()
{
    return StringToPriority(currentText());
}

}
