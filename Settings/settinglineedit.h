#ifndef SETTINGLINEEDIT_H
#define SETTINGLINEEDIT_H

#include <QLineEdit>
#include "settingbase.h"

namespace Setting
{
class SettingLineEdit : public QLineEdit, public SettingBase
{
    Q_OBJECT

public:
    explicit SettingLineEdit(QString const& name, QString const& defaultValue = "");

    // from SettingBase
    void Load(QJsonObject &object) override;
    void Save(QJsonObject &object) const override;
    void ResetDefault() override;

private:
    QString m_defaultValue;
};
}

#endif // SETTINGLINEEDIT_H
