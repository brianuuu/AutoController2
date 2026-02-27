#include "discordmanager.h"
#include "defines.h"

void DiscordManager::Initialize()
{
    m_client = new Discord::Client("bot", this);

    connect(m_settingToken, &QLineEdit::textChanged, this, &DiscordManager::OnSettingChanged);
    connect(m_btnTestUser, &QPushButton::clicked, this, &DiscordManager::OnTestUser);
    connect(m_btnTestChannel, &QPushButton::clicked, this, &DiscordManager::OnTestChannel);
    connect(m_btnStartStop, &QPushButton::clicked, this, &DiscordManager::OnToggled);
}

void DiscordManager::SetEnabled(bool enabled)
{
    if (enabled != m_enabled)
    {
        OnToggled();
    }
}

void DiscordManager::OnSettingChanged()
{
    bool const hasToken = !m_settingToken->text().isEmpty();
    m_settingToken->setEnabled(!m_enabled);
    m_settingUser->setEnabled(hasToken && !m_enabled);
    m_settingChannel->setEnabled(hasToken && !m_enabled);

    m_btnTestUser->setEnabled(m_enabled && !m_settingUser->text().isEmpty());
    m_btnTestChannel->setEnabled(m_enabled && !m_settingChannel->text().isEmpty());
    m_btnStartStop->setEnabled(hasToken);
    m_btnStartStop->setText(m_enabled ? "Stop Bot" : "Start Bot");
}

void DiscordManager::OnTestUser()
{
    if (!m_enabled || m_settingUser->text().isEmpty()) return;

    snowflake_t id = m_settingUser->text().toULongLong();
    m_client->createDm(id).then(
        [this](Discord::Channel const& c)
        {
            m_client->createMessage(c.id(), GetEmbedTemplate("Test User DM"), GetUserMention());
        }
    );
}

void DiscordManager::OnTestChannel()
{
    if (!m_enabled || m_settingChannel->text().isEmpty()) return;

    snowflake_t id = m_settingChannel->text().toULongLong();
    m_client->createMessage(id, GetEmbedTemplate("Test Channel Message"), GetUserMention());
}

void DiscordManager::OnToggled()
{
    if (m_settingToken->text().isEmpty()) return;

    if (m_enabled)
    {
        m_client->logout();
        m_enabled = false;
    }
    else
    {
        Discord::Token token;
        token.generate(m_settingToken->text(), Discord::Token::Type::BOT);
        m_client->login(token);
        m_enabled = true;
    }

    OnSettingChanged();
}

QString DiscordManager::GetUserMention() const
{
    if (m_settingUser->text().isEmpty()) return "";
    return "<@" + m_settingUser->text() + ">\n";
}

Discord::Embed DiscordManager::GetEmbedTemplate(const QString &title)
{
    Discord::Embed embed;
    embed.setTitle(title);
    embed.setDescription("By Auto Controller 2 v" + QString(VERSION) + " ([GitHub](https://github.com/brianuuu/AutoController2)/[Discord](https://discord.gg/GWEurpGZNM))");
    return embed;
}
