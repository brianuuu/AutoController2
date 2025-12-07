#ifndef SETTINGTEXTEDIT_H
#define SETTINGTEXTEDIT_H

#include <QTextEdit>
#include "Programs/settingbase.h"

class SettingTextEdit : public QTextEdit, public SettingBase
{
    Q_OBJECT

public:
    explicit SettingTextEdit(QString const& name) : SettingBase(name) {}

    // from SettingBase
    void Load(QJsonObject &object);
    void Save(QJsonObject &object) const;
    void ResetDefault();
};

#endif // SETTINGTEXTEDIT_H
