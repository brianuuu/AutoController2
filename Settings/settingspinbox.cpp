#include "settingspinbox.h"
#include "Helpers/jsonhelper.h"

namespace Setting
{
SettingSpinBox::SettingSpinBox
(
    const QString &name,
    int min,
    int max,
    int defaultValue
)
    : SettingBase(name)
    , m_defaultValue(defaultValue)
{
    this->setMinimum(min);
    this->setMaximum(max);
    this->setValue(defaultValue);
    this->setFocusPolicy(Qt::StrongFocus);
}

void SettingSpinBox::Load(QJsonObject &object)
{
    QVariant value;
    if (JsonHelper::ReadValue(object, m_name, value))
    {
        this->setValue(value.toInt());
    }
}

void SettingSpinBox::Save(QJsonObject &object) const
{
    object.insert(m_name, this->value());
}

void SettingSpinBox::ResetDefault()
{
    this->setValue(m_defaultValue);
}

void SettingSpinBox::wheelEvent(QWheelEvent *event)
{
    if (hasFocus())
    {
        QSpinBox::wheelEvent(event);
    }
    else
    {
        event->ignore();
    }
}
}
