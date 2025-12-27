#ifndef SETTINGCOLOR_H
#define SETTINGCOLOR_H

#include <QColorDialog>
#include <QPushButton>
#include "settingbase.h"

namespace Setting
{
class SettingColor : public QPushButton, public SettingBase
{
    Q_OBJECT

public:
    explicit SettingColor(QString const& name, QColor defaultColor = QColor(0,0,0));

    // from SettingBase
    void Load(QJsonObject &object) override;
    void Save(QJsonObject &object) const override;
    void ResetDefault() override;

    void SetEnabled(bool enabled);
    void SetColor(QColor color);
    QColor GetColor() const;

signals:
    void notifyColorChanged(QColor);

private slots:
    void OnClicked();

private:
    QColor m_defaultColor = QColor(0,0,0);
    QColor m_currentColor = QColor(0,0,0);
};
}

#endif // SETTINGCOLOR_H
