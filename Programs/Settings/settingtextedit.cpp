#include "settingtextedit.h"
#include "Helpers/jsonhelper.h"

void SettingTextEdit::Load(QJsonObject &object)
{
    QVariant text;
    if (JsonHelper::ReadValue(object, m_name, text))
    {
        this->setText(text.toString());
    }
}

void SettingTextEdit::Save(QJsonObject &object) const
{
    object.insert(m_name, this->toPlainText());
}

void SettingTextEdit::ResetDefault()
{
    this->clear();
}
