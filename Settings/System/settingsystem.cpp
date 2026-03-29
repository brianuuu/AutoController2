#include "settingsystem.h"

#include "Types/systemtype.h"

namespace Setting::System
{

SettingSystem::SettingSystem(const QString &name)
    : SettingComboBox(name, SystemStringList())
{

}

}
