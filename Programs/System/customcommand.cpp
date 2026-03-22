#include "customcommand.h"

#include "Helpers/jsonhelper.h"
#include "Managers/serialmanager.h"
#include "Modules/Common/runcommand.h"
#include "defines.h"

#define CUSTOM_COMMAND_DIRECTORY RESOURCES_PATH + "CustomCommand/"
#define CUSTOM_COMMAND_FORMAT QString(".customcommand")

namespace Program::System
{

CustomCommand::CustomCommand(QObject *parent) : ProgramBase(parent)
{
}

void CustomCommand::PopulateSettings(QBoxLayout *layout)
{
    m_list = new Setting::System::SettingPreset("CommandType", CUSTOM_COMMAND_DIRECTORY, CUSTOM_COMMAND_FORMAT, true);
    m_savedSettings.insert(m_list);
    AddSetting(layout, "Command Select:", "Select a pre-made command to run", m_list, true);
    connect(m_list, &QComboBox::currentTextChanged, this, &CustomCommand::OnListChanged);

    m_command = new Setting::SettingLineEdit("CommandEdit");
    m_command->setValidator(new QRegularExpressionValidator(Module::Common::RunCommand::GetRegularExpression()));
    AddSetting(layout, "Current Command:", "", m_command, false);
    connect(m_command, &QLineEdit::textChanged, this, &CustomCommand::OnCommandChanged);
    connect(m_command, &QLineEdit::textEdited, m_list, &Setting::System::SettingPreset::OnEdited);

    // add error message label and move it to the layout above, horribly
    m_labelStatus = AddText(layout, "", true);
    layout->itemAt(layout->count() - 2)->widget()->layout()->addWidget(m_labelStatus);

    m_description = new Setting::SettingTextEdit("Description");
    AddSetting(layout, "Description:", "", m_description, false);
    m_description->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(m_description, &QTextEdit::textChanged, m_list, &Setting::System::SettingPreset::OnEdited);

    m_btnSave = new QPushButton("Save As...");
    m_btnDelete = new QPushButton("Delete");
    m_btnDirectory = new QPushButton("Open Directory");
    AddSettings(layout, "", "", {m_btnSave, m_btnDelete, m_btnDirectory}, true);
    connect(m_btnSave, &QPushButton::clicked, this, &CustomCommand::OnCommandSave);
    connect(m_btnDelete, &QPushButton::clicked, m_list, &Setting::System::SettingPreset::OnDelete);
    connect(m_btnDirectory, &QPushButton::clicked, m_list, &Setting::System::SettingPreset::OnOpenDirectory);

    // set initial text
    OnListChanged(m_list->currentText());
}

bool CustomCommand::CanRun() const
{
    return ProgramBase::CanRun() && m_validCommand;
}

void CustomCommand::Start()
{
    ProgramBase::Start();
    Module::Common::RunCommand* module = new Module::Common::RunCommand(m_command->text());
    AddModule(module, true);
    m_btnDelete->setEnabled(false);
}

void CustomCommand::Stop()
{
    m_btnDelete->setEnabled(m_list->currentText() != CUSTOM_SELECTION);
    ProgramBase::Stop();
}

void CustomCommand::OnListChanged(const QString &str)
{
    m_btnDelete->setEnabled(!m_started && m_list->currentText() != CUSTOM_SELECTION);
    if (str == CUSTOM_SELECTION)
    {
        // should save custom command
        m_savedSettings.insert(m_command);
        m_savedSettings.insert(m_description);
        return;
    }
    else
    {
        // don't save
        m_savedSettings.remove(m_command);
        m_savedSettings.remove(m_description);
    }

    QString const name = CUSTOM_COMMAND_DIRECTORY + str + CUSTOM_COMMAND_FORMAT;
    QJsonObject const object = JsonHelper::ReadJson(name);

    QVariant command;
    if (JsonHelper::ReadValue(object, "Command", command))
    {
        m_command->setText(command.toString());
    }
    else
    {
        m_command->clear();
    }

    QVariant description;
    if (JsonHelper::ReadValue(object, "Description", description))
    {
        m_description->blockSignals(true);
        m_description->setText(description.toString());
        m_description->blockSignals(false);
    }
    else
    {
        m_description->clear();
    }
}

void CustomCommand::OnCommandChanged()
{
    // user input or programmatic change
    VerifyCommand();
}

void CustomCommand::OnCommandSave()
{
    QString const file = QFileDialog::getSaveFileName(m_btnSave, tr("Save Command As"), CUSTOM_COMMAND_DIRECTORY, "Custom Command (*" + CUSTOM_COMMAND_FORMAT + ")");
    if (file == Q_NULLPTR) return;

    QFileInfo const info(file);
    QString name = info.fileName();
    name = name.mid(0, name.size() - CUSTOM_COMMAND_FORMAT.size());

    if (name == CUSTOM_SELECTION)
    {
        QMessageBox::critical(m_list, "Error", "This name is not allowed", QMessageBox::Ok);
        return;
    }

    QJsonObject object;
    object.insert("Command", m_command->text());
    object.insert("Description", m_description->toPlainText());
    JsonHelper::WriteJson(file, object);

    if (m_list->findText(name) == -1)
    {
        m_list->addItem(name);
        m_list->model()->sort(0);
    }

    m_list->setCurrentText(name);
}

void CustomCommand::VerifyCommand()
{
    QString errorMsg;
    if (SerialManager::VerifyCommand(m_command->text(), errorMsg))
    {
        m_labelStatus->setText(errorMsg.isEmpty() ? "Valid!" : errorMsg);
        m_validCommand = true;
    }
    else
    {
        m_labelStatus->setText(errorMsg);
        m_validCommand = false;
    }

    QPalette palette = m_labelStatus->palette();
    palette.setColor(QPalette::WindowText, LogTypeToColor(m_validCommand ? (errorMsg.isEmpty() ? LOG_Success : LOG_Warning) : LOG_Error));
    m_labelStatus->setPalette(palette);

    m_btnSave->setEnabled(m_validCommand);
    OnCanRunChanged();
}

}
