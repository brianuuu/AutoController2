#include "settinglanguage.h"

#include "Types/languagetype.h"

namespace Setting::System
{

SettingLanguage::SettingLanguage(const QString &name)
    : SettingComboBox(name, LanguageStringList())
{

}

}
