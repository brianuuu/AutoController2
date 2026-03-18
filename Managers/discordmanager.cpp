#include "discordmanager.h"
#include "defines.h"

void DiscordManager::Initialize()
{
    m_client = new Discord::Client("bot", this);

    connect(m_settingToken, &QLineEdit::textChanged, this, &DiscordManager::OnSettingChanged);
    connect(m_settingUser, &QLineEdit::textChanged, this, &DiscordManager::OnSettingChanged);
    connect(m_settingChannel, &QLineEdit::textChanged, this, &DiscordManager::OnSettingChanged);
    connect(m_btnTestUser, &QPushButton::clicked, this, &DiscordManager::OnTestUser);
    connect(m_btnTestChannel, &QPushButton::clicked, this, &DiscordManager::OnTestChannel);
    connect(m_btnStartStop, &QPushButton::clicked, this, &DiscordManager::OnToggled);
    connect(this, &DiscordManager::notifySendMessage, this, &DiscordManager::OnSendMessage);
}

void DiscordManager::SetEnabled(bool enabled)
{
    if (enabled != m_enabled)
    {
        OnToggled();
    }
}

void DiscordManager::SendMessage(const Discord::Embed &embed, bool isMention, bool dmOnly, const QImage *img)
{
    if (!m_enabled) return;

    m_message = Message();
    m_message.embed = embed;
    m_message.isMention = isMention;
    m_message.dmOnly = dmOnly;
    m_message.image = img ? (*img) : QImage();

    if (img)
    {
        auto future = QtConcurrent::run([this]{
            m_message.u.type = Discord::UploadImageSupportedExtension::PNG;
            m_message.u.name = "attachment.png";
            QBuffer buffer(&m_message.u.file);
            buffer.open(QIODevice::WriteOnly);
            m_message.image.save(&buffer, "PNG");
            emit notifySendMessage();
        });
    }
    else
    {
        OnSendMessage();
    }
}

void DiscordManager::OnSendMessage()
{
    // mention
    QString const mention = m_message.isMention ? GetUserMention() : "";

    // send to channel
    if (!m_settingChannel->text().isEmpty() && !m_message.dmOnly)
    {
        snowflake_t id = m_settingChannel->text().toULongLong();
        if (!m_message.image.isNull())
        {
            m_client->createImageMessage(id, m_message.u, m_message.embed, mention);
        }
        else
        {
            m_client->createMessage(id, m_message.embed, mention);
        }
    }

    // send DM
    if (!m_settingUser->text().isEmpty())
    {
        snowflake_t id = m_settingUser->text().toULongLong();
        m_client->createDm(id).then(
            [this, mention](Discord::Channel const& c)
            {
                if (!m_message.image.isNull())
                {
                    m_client->createImageMessage(c.id(), m_message.u, m_message.embed, mention);
                }
                else
                {
                    m_client->createMessage(c.id(), m_message.embed, mention);
                }
            }
        );
    }
}

void DiscordManager::OnSettingChanged()
{
    m_settingToken->setEnabled(!m_enabled);

    m_btnTestUser->setEnabled(m_enabled && !m_settingUser->text().isEmpty());
    m_btnTestChannel->setEnabled(m_enabled && !m_settingChannel->text().isEmpty());
    m_btnStartStop->setEnabled(!m_settingToken->text().isEmpty());
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
    if (m_enabled)
    {
        m_client->logout();
        m_enabled = false;
    }
    else if (!m_settingToken->text().isEmpty())
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
    embed.setDescription("By Auto Controller 2 v" + VERSION + " ([GitHub](https://github.com/brianuuu/AutoController2)/[Discord](https://discord.gg/GWEurpGZNM))");
    return embed;
}
