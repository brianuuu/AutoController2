#ifndef SETTINGTEXTEDIT_H
#define SETTINGTEXTEDIT_H

#include <QTextEdit>
#include "settingbase.h"

namespace Setting
{
class SettingTextEdit : public QTextEdit, public SettingBase
{
    Q_OBJECT

public:
    explicit SettingTextEdit(QString const& name) : SettingBase(name) {}

    // from SettingBase
    void Load(QJsonObject &object) override;
    void Save(QJsonObject &object) const override;
    void ResetDefault() override;
};
}

#endif // SETTINGTEXTEDIT_H
