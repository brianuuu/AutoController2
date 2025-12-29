#include "settingsystem.h"

#include "Types/systemtype.h"

namespace Setting
{

SettingSystem::SettingSystem(const QString &name)
    : SettingComboBox(name, SystemStringList())
{

}

}
