#include "settinglanguage.h"

#include "Types/languagetype.h"

namespace Setting
{

SettingLanguage::SettingLanguage(const QString &name)
    : SettingComboBox(name, LanguageStringList())
{

}

}
