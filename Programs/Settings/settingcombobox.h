#ifndef SETTINGCOMBOBOX_H
#define SETTINGCOMBOBOX_H

#include <QComboBox>
#include "settingbase.h"

namespace Setting
{
class SettingComboBox : public QComboBox, public SettingBase
{
    Q_OBJECT

public:
    explicit SettingComboBox(QString const& name, QStringList const& list);

    // from SettingBase
    void Load(QJsonObject &object) override;
    void Save(QJsonObject &object) const override;
    void ResetDefault() override;
};
}

#endif // SETTINGCOMBOBOX_H
