#ifndef SETTINGCHECKBOX_H
#define SETTINGCHECKBOX_H

#include <QCheckBox>
#include "settingbase.h"

namespace Setting
{
class SettingCheckBox : public QCheckBox, public SettingBase
{
    Q_OBJECT

public:
    explicit SettingCheckBox(QString const& name, QString const& text = "", bool defaultValue = false);

    // from SettingBase
    void Load(QJsonObject &object) override;
    void Save(QJsonObject &object) const override;
    void ResetDefault() override;

private:
    bool m_defaultValue = false;
};
}

#endif // SETTINGCHECKBOX_H
