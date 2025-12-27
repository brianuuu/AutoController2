#include "settingcolor.h"
#include "Helpers/jsonhelper.h"

namespace Setting
{
SettingColor::SettingColor
(
    const QString &name,
    QColor defaultColor
)
    : SettingBase(name)
    , m_defaultColor(defaultColor)
{
    QFont font = this->font();
    font.setBold(true);
    this->setFont(font);

    SetColor(defaultColor);
    connect(this, &QPushButton::clicked, this, &SettingColor::OnClicked);
}

void SettingColor::Load(QJsonObject &object)
{
    QVariant rgb;
    if (JsonHelper::ReadValue(object, m_name, rgb))
    {
        SetColor(QColor(rgb.toUInt()));
    }
}

void SettingColor::Save(QJsonObject &object) const
{
    object.insert(m_name, (int)GetColor().rgb());
}

void SettingColor::ResetDefault()
{
    SetColor(m_defaultColor);
}

void SettingColor::SetEnabled(bool enabled)
{
    if (enabled)
    {
        SetColor(m_currentColor);
    }
    else
    {
        QString const qss = QString("background-color: %1; color: %2").arg(QColor(Qt::lightGray).name(), QColor(Qt::black).name());
        this->setStyleSheet(qss);
    }

    this->setText(enabled ? m_currentColor.name().toUpper() : "Disabled");
    this->setEnabled(enabled);
}

void SettingColor::SetColor(QColor color)
{
    m_currentColor = color;
    if (!this->isEnabled()) return;

    QString const qss = QString("background-color: %1; color: %2").arg(color.name(), QColor(color.valueF() < 0.5f ? Qt::white : Qt::black).name());
    this->setStyleSheet(qss);
    this->setText(color.name().toUpper());
}

QColor SettingColor::GetColor() const
{
    return m_currentColor;
}

void SettingColor::OnClicked()
{
    QColor const color = QColorDialog::getColor(GetColor(), Q_NULLPTR, "Select Color");
    if (color.isValid() && color != GetColor())
    {
        SetColor(color);
        emit notifyColorChanged(color);
    }
}
}
