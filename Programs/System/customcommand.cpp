#include "customcommand.h"

#include "Helpers/jsonhelper.h"
#include "Managers/audiomanager.h"
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

    m_btnPlay = new QPushButton("Play");
    m_btnPlay->setEnabled(false);
    m_sound = new Setting::System::SettingPreset("SoundType", AudioManager::GetDirectory(), AudioManager::GetExtension(), false);
    AddSettings(layout, "Sound Detection:", "If this sound is detected, a capture is taken and stops the program at Home screen", {m_btnPlay, m_sound}, true);
    connect(m_btnPlay, &QPushButton::clicked, this, &CustomCommand::OnPlaySound);
    connect(m_sound, &QComboBox::currentTextChanged, this, &CustomCommand::OnSoundChanged);
    m_btnPlay->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    m_command = new Setting::System::SettingCommand("CommandEdit", false);
    AddSetting(layout, "Current Command:", "", m_command, false);
    connect(m_command, &Setting::System::SettingCommand::notifyValid, this, &CustomCommand::OnCommandChanged);
    connect(m_command, &Setting::System::SettingCommand::notifyEdited, m_list, &Setting::System::SettingPreset::OnEdited);

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
    Module::Common::RunCommand* module = new Module::Common::RunCommand(m_command->GetText());
    m_moduleHolder->AddModule(module, true);
    m_btnDelete->setEnabled(false);

    if (m_sound->currentIndex() > 0)
    {
        m_soundID = m_audioManager->AddDetection(m_sound->currentText());
        if (m_soundID == 0)
        {
            emit notifyFinished(false);
        }
        else
        {
            m_audioManager->StartDetection(m_soundID);
        }
    }
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
        m_savedSettings.insert(m_sound);
        m_savedSettings.insert(m_command);
        m_savedSettings.insert(m_description);
        return;
    }
    else
    {
        // don't save
        m_savedSettings.remove(m_sound);
        m_savedSettings.remove(m_command);
        m_savedSettings.remove(m_description);
    }

    QString const name = CUSTOM_COMMAND_DIRECTORY + str + CUSTOM_COMMAND_FORMAT;
    QJsonObject const object = JsonHelper::ReadJson(name);

    QVariant sound;
    if (JsonHelper::ReadValue(object, "Sound", sound))
    {
        m_sound->setCurrentText(sound.toString());
    }
    else
    {
        m_sound->setCurrentIndex(0);
    }

    QVariant command;
    if (JsonHelper::ReadValue(object, "Command", command))
    {
        m_command->SetText(command.toString());
    }
    else
    {
        m_command->SetText("");
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

void CustomCommand::OnSoundChanged()
{
    m_btnPlay->setEnabled(m_sound->currentIndex() > 0);
    m_list->OnEdited();
    OnCanRunChanged();
}

void CustomCommand::OnPlaySound()
{
    if (m_mediaPlayer)
    {
        m_mediaPlayer->stop();
        delete m_mediaPlayer;
    }

    QString const name = AudioManager::GetDirectory() + m_sound->currentText() + AudioManager::GetExtension();
    QJsonObject const object = JsonHelper::ReadJson(name);
    if (object.isEmpty()) return;

    QVariant value;
    if (!JsonHelper::ReadValue(object, "File", value)) return;

    m_mediaPlayer = new QMediaPlayer(this);
    m_mediaPlayer->setAudioOutput(m_audioManager->GetAudioOutput());
    m_mediaPlayer->setSource(QUrl::fromLocalFile(RESOURCES_PATH + value.toString() + ".wav"));
    m_mediaPlayer->play();
}

void CustomCommand::OnCommandChanged(bool valid)
{
    // user input or programmatic change
    m_validCommand = valid;
    m_btnSave->setEnabled(m_validCommand);
    OnCanRunChanged();
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
    object.insert("Sound", m_sound->currentText());
    object.insert("Command", m_command->GetText());
    object.insert("Description", m_description->toPlainText());
    JsonHelper::WriteJson(file, object);

    if (m_list->findText(name) == -1)
    {
        m_list->addItem(name);
        m_list->model()->sort(0);
    }

    m_list->setCurrentText(name);
}

void CustomCommand::OnCommandFinished(Module::Common::RunCommand* module)
{
    // only used when sound was detected
    emit notifyFinished(true);
}

void CustomCommand::OnSoundDetected(int id)
{
    // interrupt current command
    m_moduleHolder->ClearModules();
    m_moduleHolder->AddRunCommand("System_CaptureHome", 0);
}

}
