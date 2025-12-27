#ifndef SETTINGLANGUAGE_H
#define SETTINGLANGUAGE_H

#include "settingcombobox.h"

namespace Setting
{
class SettingLanguage : public SettingComboBox
{
    Q_OBJECT
public:
    explicit SettingLanguage(QString const& name);
};
}

#endif // SETTINGLANGUAGE_H
