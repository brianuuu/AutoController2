#ifndef DISCORDMANAGER_H
#define DISCORDMANAGER_H

#include <QBuffer>
#include <QPushButton>
#include <QtConcurrent>
#include <QWidget>

#include "External/QDiscord/Discord/Client.h"
#include "Settings/settingcheckbox.h"
#include "Settings/settinglineedit.h"
#include "Settings/settingspinbox.h"

namespace Ui { class MainWindow; }

class DiscordManager : public QWidget
{
    Q_OBJECT
public:
    explicit DiscordManager(QWidget *parent = nullptr) : QWidget(parent) {}
    static QString GetTypeID() { return "Discord"; }
    void Initialize();

    bool IsEnabled() const { return m_enabled; }
    void SetEnabled(bool enabled);

    static Discord::Embed GetEmbedTemplate(QString const& title);
    void SendMessage(const Discord::Embed &embed, bool isMention, bool dmOnly, const QImage *img = Q_NULLPTR);

public:
    // Setting
    Setting::SettingLineEdit* m_settingToken = Q_NULLPTR;
    Setting::SettingLineEdit* m_settingUser = Q_NULLPTR;
    Setting::SettingLineEdit* m_settingChannel = Q_NULLPTR;
    Setting::SettingCheckBox* m_settingHourlyUpdate = Q_NULLPTR;
    Setting::SettingSpinBox* m_settingFinishSuppress = Q_NULLPTR;
    QPushButton* m_btnTestUser = Q_NULLPTR;
    QPushButton* m_btnTestChannel = Q_NULLPTR;
    QPushButton* m_btnStartStop = Q_NULLPTR;

signals:
    void notifySendMessage();

private slots:
    void OnSettingChanged();
    void OnTestUser();
    void OnTestChannel();
    void OnToggled();
    void OnSendMessage();

private:
    QString GetUserMention() const;

private:
    // QDiscord
    Discord::Client* m_client;
    bool m_enabled = false;

    struct Message
    {
        Discord::Embed embed;
        Discord::UploadAttachment u;
        QImage image;
        bool isMention = false;
        bool dmOnly = false;
    } m_message;
};

#endif // DISCORDMANAGER_H
