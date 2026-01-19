#include "settingtextbrowser.h"
#include "Helpers/jsonhelper.h"

namespace Setting
{
SettingTextBrowser::SettingTextBrowser
(
    const QString &name,
    const QString &defaultText
)
    : SettingBase(name)
    , m_defaultText(defaultText)
{
    this->setPlainText(m_defaultText);
}

void SettingTextBrowser::Load(QJsonObject &object)
{
    QVariant text;
    if (JsonHelper::ReadValue(object, m_name, text))
    {
        this->setPlainText(text.toString());
    }
}

void SettingTextBrowser::Save(QJsonObject &object) const
{
    object.insert(m_name, this->toPlainText());
}

void SettingTextBrowser::ResetDefault()
{
    this->setPlainText(m_defaultText);
}
}
