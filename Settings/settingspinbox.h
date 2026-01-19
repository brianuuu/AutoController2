#ifndef SETTINGSPINBOX_H
#define SETTINGSPINBOX_H

#include <QSpinBox>
#include "settingbase.h"

namespace Setting
{
class SettingSpinBox : public QSpinBox, public SettingBase
{
    Q_OBJECT

public:
    explicit SettingSpinBox(QString const& name, int min, int max, int defaultValue = 0);

    // from SettingBase
    void Load(QJsonObject &object) override;
    void Save(QJsonObject &object) const override;
    void ResetDefault() override;

private:
    int m_defaultValue = 0;
};
}

#endif // SETTINGSPINBOX_H
