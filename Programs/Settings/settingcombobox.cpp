#include "settingcombobox.h"
#include "Helpers/jsonhelper.h"

SettingComboBox::SettingComboBox
(
    const QString &name,
    const QStringList &list
)
    : SettingBase(name)
{
    this->addItems(list);
}

void SettingComboBox::Load(QJsonObject &object)
{
    QVariant text;
    if (JsonHelper::ReadValue(object, m_name, text))
    {
        this->setCurrentText(text.toString());
    }
}

void SettingComboBox::Save(QJsonObject &object) const
{
    object.insert(m_name, this->currentText());
}

void SettingComboBox::ResetDefault()
{
    this->setCurrentIndex(0);
}
