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
        m_commandSettings[i] = new Setting::System::SettingCommand(name, i != ST_COUNT);
        AddSetting(layout, fullName + " Command:", "", m_commandSettings[i], false);
        connect(m_commandSettings[i], &Setting::System::SettingCommand::notifyValid, this, &DevCommand::OnCommandChanged);
        connect(m_commandSettings[i], &Setting::System::SettingCommand::notifyEdited, m_list, &Setting::System::SettingPreset::OnEdited);
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
        QString const systemCommand = m_commandSettings[systemType]->GetText();
        module = new Module::Common::RunCommand(systemCommand.isEmpty() ? m_commandSettings[ST_COUNT]->GetText() : systemCommand);
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
        for (auto& command : m_commandSettings)
        {
            m_savedSettings.insert(command);
        }
        return;
    }
    else
    {
        // don't save
        for (auto& command : m_commandSettings)
        {
            m_savedSettings.remove(command);
        }
    }

    QString const name = Module::Common::RunCommand::GetDirectory() + str + Module::Common::RunCommand::GetExtension();
    QJsonObject const object = JsonHelper::ReadJson(name);

    for (int i = 0; i <= ST_COUNT; i++)
    {
        QString const key = i == ST_COUNT ? "Default" : SystemToString((SystemType)i);

        QVariant command;
        if (JsonHelper::ReadValue(object, key, command))
        {
            m_commandSettings[i]->SetText(command.toString());
        }
        else
        {
            m_commandSettings[i]->SetText("");
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
        QString const key = i == ST_COUNT ? "Default" : SystemToString((SystemType)i);
        object.insert(key, m_commandSettings[i]->GetText());
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
        m_validCommand &= m_commandSettings[i]->IsValid();
    }

    m_btnSave->setEnabled(m_validCommand);
    OnCanRunChanged();
}

}
