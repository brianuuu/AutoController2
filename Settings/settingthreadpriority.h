#ifndef SETTINGTHREADPRIORITY_H
#define SETTINGTHREADPRIORITY_H

#include <QThread>
#include "settingcombobox.h"

namespace Setting
{

class SettingThreadPriority : public SettingComboBox
{
    Q_OBJECT

public:
    explicit SettingThreadPriority(QString const& name, QThread::Priority defaultPriority);

    // from SettingBase
    void ResetDefault() override;

    static QString PriorityToString(QThread::Priority priority);
    static QThread::Priority StringToPriority(QString const& str);
    static QStringList GetAvailableList();

    QThread::Priority GetPriority();

private:
    QThread::Priority m_defaultPriority = QThread::Priority::InheritPriority;
};

}

#endif // SETTINGTHREADPRIORITY_H
