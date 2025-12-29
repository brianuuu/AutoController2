#ifndef SETTINGSYSTEM_H
#define SETTINGSYSTEM_H

#include "settingcombobox.h"

namespace Setting
{
class SettingSystem : public SettingComboBox
{
    Q_OBJECT
public:
    explicit SettingSystem(QString const& name);
};
}

#endif // SETTINGSYSTEM_H
