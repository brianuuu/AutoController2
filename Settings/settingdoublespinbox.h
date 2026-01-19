#ifndef SETTINGDOUBLESPINBOX_H
#define SETTINGDOUBLESPINBOX_H

#include <QDoubleSpinBox>
#include "settingbase.h"

namespace Setting
{
class SettingDoubleSpinBox : public QDoubleSpinBox, public SettingBase
{
    Q_OBJECT

public:
    explicit SettingDoubleSpinBox(QString const& name, double min, double max, double defaultValue = 0);

    // from SettingBase
    void Load(QJsonObject &object) override;
    void Save(QJsonObject &object) const override;
    void ResetDefault() override;

private:
    double m_defaultValue = 0.0;
};
}

#endif // SETTINGDOUBLESPINBOX_H
