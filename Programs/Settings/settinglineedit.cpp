#include "settinglineedit.h"
#include "Helpers/jsonhelper.h"

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
    this->clear();
}
