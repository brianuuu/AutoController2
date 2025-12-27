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
    this->setEnabled(enabled);
    this->setText(enabled ? m_currentColor.name().toUpper() : "Disabled");

    if (enabled)
    {
        SetColor(m_currentColor);
    }
    else
    {
        QString const qss = QString("background-color: %1; color: %2").arg(QColor(Qt::lightGray).name(), QColor(Qt::black).name());
        this->setStyleSheet(qss);
    }
}

void SettingColor::SetColor(QColor color)
{
    m_currentColor = color;
    if (!this->isEnabled()) return;

    QColor const textColor = QColor((color.valueF() < 0.5f || (color.hue() > 200 && color.hue() < 280 && color.saturationF() >= 0.5f)) ? Qt::white : Qt::black);
    QString const qss = QString("background-color: %1; color: %2").arg(color.name(), textColor.name());
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
