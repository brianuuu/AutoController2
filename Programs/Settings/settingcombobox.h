#ifndef SETTINGCOMBOBOX_H
#define SETTINGCOMBOBOX_H

#include <QComboBox>
#include "Programs/settingbase.h"

class SettingComboBox : public QComboBox, public SettingBase
{
    Q_OBJECT

public:
    explicit SettingComboBox(QString const& name, QStringList const& list);

    // from SettingBase
    void Load(QJsonObject &object);
    void Save(QJsonObject &object) const;
};

#endif // SETTINGCOMBOBOX_H
