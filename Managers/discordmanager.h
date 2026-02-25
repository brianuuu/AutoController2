#ifndef DISCORDMANAGER_H
#define DISCORDMANAGER_H

#include <QWidget>

#include "Settings/settinglineedit.h"

namespace Ui { class MainWindow; }

class DiscordManager : public QWidget
{
    Q_OBJECT
public:
    explicit DiscordManager(QWidget *parent = nullptr) : QWidget(parent) {}
    static QString GetTypeID() { return "Discord"; }
    void Initialize();

public:
    // Setting
    Setting::SettingLineEdit* m_settingToken = Q_NULLPTR;
    Setting::SettingLineEdit* m_settingUser = Q_NULLPTR;
    Setting::SettingLineEdit* m_settingChannel = Q_NULLPTR;
};

#endif // DISCORDMANAGER_H
