#include "settingcheckbox.h"
#include "Helpers/jsonhelper.h"

namespace Setting
{
SettingCheckBox::SettingCheckBox
(
    const QString &name,
    const QString &text,
    bool defaultValue
)
    : SettingBase(name)
    , m_defaultValue(defaultValue)
{
    this->setText(text);
    this->setChecked(defaultValue);
}

void SettingCheckBox::Load(QJsonObject &object)
{
    QVariant value;
    if (JsonHelper::ReadValue(object, m_name, value))
    {
        this->setChecked(value.toBool());
    }
}

void SettingCheckBox::Save(QJsonObject &object) const
{
    object.insert(m_name, this->isChecked());
}

void SettingCheckBox::ResetDefault()
{
    this->setChecked(m_defaultValue);
}
}
