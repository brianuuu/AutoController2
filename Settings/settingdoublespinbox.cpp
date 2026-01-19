#include "settingdoublespinbox.h"
#include "Helpers/jsonhelper.h"

namespace Setting
{
SettingDoubleSpinBox::SettingDoubleSpinBox
(
    const QString &name,
    double min,
    double max,
    double defaultValue
)
    : SettingBase(name)
    , m_defaultValue(defaultValue)
{
    this->setMinimum(min);
    this->setMaximum(max);
    this->setValue(defaultValue);
}

void SettingDoubleSpinBox::Load(QJsonObject &object)
{
    QVariant value;
    if (JsonHelper::ReadValue(object, m_name, value))
    {
        this->setValue(value.toDouble());
    }
}

void SettingDoubleSpinBox::Save(QJsonObject &object) const
{
    object.insert(m_name, this->value());
}

void SettingDoubleSpinBox::ResetDefault()
{
    this->setValue(m_defaultValue);
}
}
