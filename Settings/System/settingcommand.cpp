#include "settingcommand.h"

#include "Helpers/jsonhelper.h"
#include "Programs/programbase.h"
#include "Managers/serialmanager.h"
#include "Modules/Common/runcommand.h"

namespace Setting::System {

SettingCommand::SettingCommand(const QString &name, bool allowEmpty)
    : SettingBase(name)
    , m_allowEmpty(allowEmpty)
{
    QVBoxLayout* vBoxLayout = new QVBoxLayout(this);
    vBoxLayout->setContentsMargins(0,0,0,0);
    vBoxLayout->setSpacing(0);

    m_command = new QLineEdit();
    m_command->setValidator(new QRegularExpressionValidator(Module::Common::RunCommand::GetRegularExpression()));
    connect(m_command, &QLineEdit::textChanged, this, &SettingCommand::VerifyCommand);
    connect(m_command, &QLineEdit::textEdited, this, &SettingCommand::notifyEdited);
    vBoxLayout->addWidget(m_command);

    m_status = Program::ProgramBase::AddText(vBoxLayout, "", true);
    VerifyCommand();
}

void SettingCommand::Load(QJsonObject &object)
{
    QVariant text;
    if (JsonHelper::ReadValue(object, m_name, text))
    {
        m_command->setText(text.toString());
    }
}

void SettingCommand::Save(QJsonObject &object) const
{
    object.insert(m_name, m_command->text());
}

void SettingCommand::ResetDefault()
{
    m_command->clear();
}

void SettingCommand::VerifyCommand()
{
    QString errorMsg;
    if (m_command->text().isEmpty() && m_allowEmpty)
    {
        m_status->setText("Valid!");
        m_valid = true;
    }
    else if (SerialManager::VerifyCommand(m_command->text(), errorMsg))
    {
        int const duration = SerialManager::GetCommandDuration(m_command->text());
        QString text = errorMsg.isEmpty() ? "Valid!" : errorMsg;
        text += " (Duration: " + (duration > 0 ? QString::number(duration) + "ms)" : "Indefinite)");
        m_status->setText(text);
        m_valid = true;
    }
    else
    {
        m_status->setText(errorMsg);
        m_valid = false;
    }

    QPalette palette = m_status->palette();
    palette.setColor(QPalette::WindowText, LogTypeToColor(m_valid ? (errorMsg.isEmpty() ? LOG_Success : LOG_Warning) : LOG_Error));
    m_status->setPalette(palette);

    emit notifyValid(m_valid);
}

}
