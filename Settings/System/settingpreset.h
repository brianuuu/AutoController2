#ifndef SETTINGPRESET_H
#define SETTINGPRESET_H

#include <QDesktopServices>
#include <QDir>
#include <QMessageBox>

#include "../settingcombobox.h"

namespace Setting::System
{
class SettingPreset : public SettingComboBox
{
    Q_OBJECT
public:
    explicit SettingPreset(QString const& name, QString const& directory, QString const& extension, bool addCustom);

public slots:
    void OnEdited();
    void OnDelete();
    void OnOpenDirectory();

private:
    QString m_directory;
    QString m_extension;
};
}

#endif // SETTINGPRESET_H
