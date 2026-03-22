#include "devcommand.h"

#include "Helpers/jsonhelper.h"
#include "Managers/profilemanager.h"
#include "Managers/serialmanager.h"
#include "Modules/Common/runcommand.h"
#include "Types/systemtype.h"
#include "defines.h"

namespace Program::Development
{

DevCommand::DevCommand(QObject *parent) : ProgramBase(parent)
{

}

void DevCommand::PopulateSettings(QBoxLayout *layout)
{
    m_list = new Setting::System::SettingPreset("CommandType", Module::Common::RunCommand::GetDirectory(), Module::Common::RunCommand::GetExtension(), true);
    m_savedSettings.insert(m_list);
    AddSetting(layout, "Command Select:", "Select command to run", m_list, true);
    connect(m_list, &QComboBox::currentTextChanged, this, &DevCommand::OnListChanged);

    m_commandSettings.resize(ST_COUNT + 1);
    for (int i = ST_COUNT; i >= 0; i--)
    {
        QString const name = i == ST_COUNT ? "Default" : SystemToString((SystemType)i);
        QString const fullName = i == ST_COUNT ? "Default" : SystemToFullString((SystemType)i);
        CommandSettings& settings = m_commandSettings[i];

        settings.m_command = new Setting::SettingLineEdit(name);
        settings.m_command->setValidator(new QRegularExpressionValidator(Module::Common::RunCommand::GetRegularExpression()));
        AddSetting(layout, fullName + " Command:", "", settings.m_command, false);
        connect(settings.m_command, &QLineEdit::textChanged, this, &DevCommand::OnCommandChanged);
        connect(settings.m_command, &QLineEdit::textEdited, m_list, &Setting::System::SettingPreset::OnEdited);

        // add error message label and move it to the layout above, horribly
        settings.m_labelStatus = AddText(layout, "", true);
        layout->itemAt(layout->count() - 2)->widget()->layout()->addWidget(settings.m_labelStatus);
    }

    m_btnSave = new QPushButton("Save As...");
    m_btnDelete = new QPushButton("Delete");
    m_btnDirectory = new QPushButton("Open Directory");
    AddSettings(layout, "", "", {m_btnSave, m_btnDelete, m_btnDirectory}, true);
    connect(m_btnSave, &QPushButton::clicked, this, &DevCommand::OnCommandSave);
    connect(m_btnDelete, &QPushButton::clicked, m_list, &Setting::System::SettingPreset::OnDelete);
    connect(m_btnDirectory, &QPushButton::clicked, m_list, &Setting::System::SettingPreset::OnOpenDirectory);

    AddSpacer(layout);

    // set initial text
    OnListChanged(m_list->currentText());
}

bool DevCommand::CanRun() const
{
    return ProgramBase::CanRun() && m_validCommand;
}

void DevCommand::Start()
{
    ProgramBase::Start();

    Module::Common::RunCommand* module = Q_NULLPTR;
    if (m_list->currentText() == CUSTOM_SELECTION)
    {
        SystemType const systemType = m_profileManager->GetSystemType();
        QString const systemCommand = m_commandSettings[systemType].m_command->text();
        module = new Module::Common::RunCommand(systemCommand.isEmpty() ? m_commandSettings[ST_COUNT].m_command->text() : systemCommand);
    }
    else
    {
        module = new Module::Common::RunCommand(m_list->currentText(), 0);
    }

    AddModule(module, true);
    m_btnDelete->setEnabled(false);
}

void DevCommand::Stop()
{
    m_btnDelete->setEnabled(m_list->currentText() != CUSTOM_SELECTION);
    ProgramBase::Stop();
}

void DevCommand::OnListChanged(const QString &str)
{
    m_btnDelete->setEnabled(!m_started && m_list->currentText() != CUSTOM_SELECTION);
    if (str == CUSTOM_SELECTION)
    {
        // should save custom command
        for (CommandSettings& settings : m_commandSettings)
        {
            m_savedSettings.insert(settings.m_command);
        }
        return;
    }
    else
    {
        // don't save
        for (CommandSettings& settings : m_commandSettings)
        {
            m_savedSettings.remove(settings.m_command);
        }
    }

    QString const name = Module::Common::RunCommand::GetDirectory() + str + Module::Common::RunCommand::GetExtension();
    QJsonObject const object = JsonHelper::ReadJson(name);

    for (int i = 0; i <= ST_COUNT; i++)
    {
        CommandSettings& settings = m_commandSettings[i];
        QString const key = i == ST_COUNT ? "Default" : SystemToString((SystemType)i);

        QVariant command;
        if (JsonHelper::ReadValue(object, key, command))
        {
            settings.m_command->setText(command.toString());
        }
        else
        {
            settings.m_command->clear();
        }
    }
}

void DevCommand::OnCommandChanged()
{
    // user input or programmatic change
    VerifyCommand();
}

void DevCommand::OnCommandSave()
{
    QString const file = QFileDialog::getSaveFileName(m_btnSave, tr("Save Command As"), Module::Common::RunCommand::GetDirectory(), "Command (*" + Module::Common::RunCommand::GetExtension() + ")");
    if (file == Q_NULLPTR) return;

    QFileInfo const info(file);
    QString name = info.fileName();
    name = name.mid(0, name.size() - Module::Common::RunCommand::GetExtension().size());

    if (name == CUSTOM_SELECTION)
    {
        QMessageBox::critical(m_list, "Error", "This name is not allowed", QMessageBox::Ok);
        return;
    }

    QJsonObject object;
    for (int i = 0; i <= ST_COUNT; i++)
    {
        CommandSettings const& settings = m_commandSettings[i];
        QString const key = i == ST_COUNT ? "Default" : SystemToString((SystemType)i);
        object.insert(key, settings.m_command->text());
    }
    JsonHelper::WriteJson(file, object);

    if (m_list->findText(name) == -1)
    {
        m_list->addItem(name);
        m_list->model()->sort(0);
    }

    m_list->setCurrentText(name);
}

void DevCommand::VerifyCommand()
{
    m_validCommand = true;

    for (int i = 0; i < m_commandSettings.size(); i++)
    {
        QString errorMsg;
        bool valid = true;

        CommandSettings& settings = m_commandSettings[i];
        if (i < ST_COUNT && settings.m_command->text().isEmpty())
        {
            // Default cannot be empty
            settings.m_labelStatus->setText("Valid!");
        }
        else if (SerialManager::VerifyCommand(settings.m_command->text(), errorMsg))
        {
            settings.m_labelStatus->setText(errorMsg.isEmpty() ? "Valid!" : errorMsg);
        }
        else
        {
            settings.m_labelStatus->setText(errorMsg);
            m_validCommand = false;
            valid = false;
        }

        QPalette palette = settings.m_labelStatus->palette();
        palette.setColor(QPalette::WindowText, LogTypeToColor(valid ? (errorMsg.isEmpty() ? LOG_Success : LOG_Warning) : LOG_Error));
        settings.m_labelStatus->setPalette(palette);
    }

    m_btnSave->setEnabled(m_validCommand);
    OnCanRunChanged();
}

}
