#include "settinglineedit.h"
#include "Helpers/jsonhelper.h"

namespace Setting
{
SettingLineEdit::SettingLineEdit
(
    const QString &name,
    const QString &defaultValue
)
    : SettingBase(name)
    , m_defaultValue(defaultValue)
{
    this->setText(defaultValue);
}

void SettingLineEdit::Load(QJsonObject &object)
{
    QVariant text;
    if (JsonHelper::ReadValue(object, m_name, text))
    {
        this->setText(text.toString());
    }
}

void SettingLineEdit::Save(QJsonObject &object) const
{
    object.insert(m_name, this->text());
}

void SettingLineEdit::ResetDefault()
{
    this->setText(m_defaultValue);
}
}
