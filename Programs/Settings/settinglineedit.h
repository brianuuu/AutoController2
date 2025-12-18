#ifndef SETTINGLINEEDIT_H
#define SETTINGLINEEDIT_H

#include <QLineEdit>
#include "settingbase.h"

class SettingLineEdit : public QLineEdit, public SettingBase
{
    Q_OBJECT

public:
    explicit SettingLineEdit(QString const& name) : SettingBase(name) {}

    // from SettingBase
    void Load(QJsonObject &object);
    void Save(QJsonObject &object) const;
    void ResetDefault();
};

#endif // SETTINGLINEEDIT_H
