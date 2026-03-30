#include "rngmanipulation.h"

namespace Program::PokemonFRLG
{

void RNGManipulation::PopulateSettings(QBoxLayout *layout)
{
    m_tid = new Setting::SettingSpinBox("TID", 0, 99999);
    m_tid->m_noReset = true;
    m_savedSettings.insert(m_tid);
    AddSetting(layout, "Trainer ID:", "Saves your TID, does not affect the program\n(Ignores Restoring Default Settings)", m_tid);

    m_sid = new Setting::SettingSpinBox("SID", 0, 99999);
    m_sid->m_noReset = true;
    m_savedSettings.insert(m_sid);
    AddSetting(layout, "Secret ID:", "Saves your SID, does not affect the program\n(Ignores Restoring Default Settings)", m_sid);

    m_seedTime = new Setting::SettingSpinBox("SeedTime", 29500, INT_MAX, 0);
    m_savedSettings.insert(m_seedTime);
    AddSetting(layout, "Seed Time:", "How many ms to wait from Home screen until pressing A at title screen", m_seedTime);

    m_seedCalibrate = new Setting::SettingSpinBox("SeedCalibrate", -INT_MAX, INT_MAX, 0);
    m_seedHit = new QLineEdit();
    m_seedHit->setPlaceholderText("Enter Hit Seed");
    m_seedHit->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]*")));
    m_btnSeedUpdate = new QPushButton("Update");
    m_savedSettings.insert(m_seedCalibrate);
    AddSettings(layout, "Seed Calibrate:", "How many ms off from your selected seed and hit seed, set the hit seed on the right and press Update", {m_seedCalibrate, m_seedHit, m_btnSeedUpdate});
    m_btnSeedUpdate->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    connect(m_seedTime, &QSpinBox::valueChanged, this, &RNGManipulation::OnCanRunChanged);
    connect(m_seedCalibrate, &QSpinBox::valueChanged, this, &RNGManipulation::OnCanRunChanged);
    connect(m_btnSeedUpdate, &QPushButton::pressed, this, &RNGManipulation::OnUpdateSeedCalibrate);

    m_continueFrames = new Setting::SettingSpinBox("ContinueFrames", 190, INT_MAX, 0);
    m_savedSettings.insert(m_continueFrames);
    AddSetting(layout, "Continue Screen Frames:", "How many frames to wait at continue screen before pressing A, 1 advance per frame", m_continueFrames);

    m_continueCalibrate = new Setting::SettingSpinBox("ContinueCalibrate", -INT_MAX, INT_MAX, 0);
    m_continueHit = new QLineEdit();
    m_continueHit->setPlaceholderText("Enter Hit Frame");
    m_continueHit->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]*")));
    m_btnContinueUpdate = new QPushButton("Update");
    m_savedSettings.insert(m_continueCalibrate);
    AddSettings(layout, "Continue Calibrate:", "How many frames off from your selected continue screen frames and hit frame, set the hit frame on the right and press Update", {m_continueCalibrate, m_continueHit, m_btnContinueUpdate});
    m_btnContinueUpdate->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    connect(m_continueFrames, &QSpinBox::valueChanged, this, &RNGManipulation::OnCanRunChanged);
    connect(m_continueCalibrate, &QSpinBox::valueChanged, this, &RNGManipulation::OnCanRunChanged);
    connect(m_btnContinueUpdate, &QPushButton::pressed, this, &RNGManipulation::OnUpdateContinueCalibrate);

    m_overworldFrames = new Setting::SettingSpinBox("OverworldFrames", 230, INT_MAX, 600);
    m_savedSettings.insert(m_overworldFrames);
    AddSetting(layout, "Overworld Frames:", "How many frames to wait in the overworld before pressing A, 2 advances per frame", m_overworldFrames);

    m_moveUp = new Setting::SettingCheckBox("MoveUp");
    m_savedSettings.insert(m_moveUp);
    AddSetting(layout, "Move Up instead of Press A:", "Replaces final A press with Up move, for Ho-Oh only", m_moveUp);

    m_commandFlashback = new Setting::System::SettingCommand("CommandFlashback", true);
    m_savedSettings.insert(m_commandFlashback);
    AddSetting(layout, "Command after Flashback:", "(Optional) Command used after flashback, use this for going through gift Pokemon dialogues or head to Sweet Scent button, the time to complete this command must be less than overworld frames. Use Command Recorder program to record this", m_commandFlashback, false);
    connect(m_commandFlashback, &Setting::System::SettingCommand::notifyValid, this, &ProgramBase::OnCanRunChanged);

    m_commandComplete = new Setting::System::SettingCommand("CommandComplete", true);
    m_savedSettings.insert(m_commandComplete);
    AddSetting(layout, "Command after Pokemon:", "(Optional) Command used after the final A press, you can use this to catch Pokemon with Master Ball or navigate to Pokemon summary screen. Use Command Recorder program to record this", m_commandComplete, false);
    connect(m_commandComplete, &Setting::System::SettingCommand::notifyValid, this, &ProgramBase::OnCanRunChanged);

    AddSpacer(layout);
}

bool RNGManipulation::CanRun() const
{
    return ProgramBase::CanRun() && m_commandFlashback->IsValid() && m_commandComplete->IsValid()
           && (m_seedTime->value() + m_seedCalibrate->value()) > 0
           && (m_continueFrames->value() + m_continueCalibrate->value()) > 0;
}

void RNGManipulation::Start()
{
    ProgramBase::Start();

    int const waitTime = m_seedTime->value() + m_seedCalibrate->value();
    m_timer.start(waitTime);

    m_state = SetState(State::TitleScreen, "Waiting for " + QString::number(waitTime) + "ms at Title Screen");
    m_moduleHolder->AddRunCommand("A|100");
}

void RNGManipulation::Stop()
{
    ProgramBase::Stop();
}

void RNGManipulation::OnCommandFinished(Module::Common::RunCommand* module)
{
    if (OnModuleErrorQuit(module)) return;
    m_moduleHolder->ClearModule(module);
	
	switch (m_state)
    {
    case State::TitleScreen:
    case State::ContinueScreen:
    case State::CommandFlashback:
    {
        // nothing, waiting for timeout
        break;
    }
    case State::Flashback:
    {
        m_state = SetState(State::CommandFlashback, "Running user command after flashback");
        m_moduleHolder->AddRunCommand(m_commandFlashback->GetText());
        break;
    }
    case State::CommandComplete:
    {
        emit notifyFinished(true);
        break;
    }
    default:
    {
        UnhandedStateRunCommand();
        return;
    }
    }
}

void RNGManipulation::OnWaitTimeout()
{
    switch (m_state)
    {
    case State::TitleScreen:
    {
        int const advanceFrames = m_continueFrames->value() + m_continueCalibrate->value();
        m_timer.start(advanceFrames * 1000 / 60);

        m_state = SetState(State::ContinueScreen, "Hold A for 3s (180F) then wait " + QString::number(advanceFrames - 180) + "F at Continue Screen");
        m_moduleHolder->AddRunCommand("A|3000");
        break;
    }
    case State::ContinueScreen:
    {
        int const overworldFrames = m_overworldFrames->value();
        m_timer.start(overworldFrames * 1000 / 60);

        m_state = SetState(m_commandFlashback->GetText().isEmpty() ? State::CommandFlashback : State::Flashback, "Skipping flashback");
        m_moduleHolder->AddRunCommand("FRLG_ContinueGame", 0);
        break;
    }
    case State::Flashback:
    {
        emit notifyFinished(false, "Overworld Frames is not enough after running user command");
        break;
    }
    case State::CommandFlashback:
    {
        if (m_moduleHolder->HasRunCommand())
        {
            emit notifyFinished(false, "Flashback command not finished before final action, please increase Overworld Frames");
            return;
        }

        m_state = SetState(State::CommandComplete, "Final action (A/Up press) and running user command if provided");
        QString const userCommand = m_commandComplete->GetText();
        m_moduleHolder->AddRunCommand((m_moveUp->isChecked() ? "LUp|50,None|50" : "A|50,None|50") + (userCommand.isEmpty() ? "" : "," + userCommand));
        break;
    }
    default:
    {
        emit notifyFinished(false);
        break;
    }
    }
}

void RNGManipulation::OnUpdateSeedCalibrate()
{
    QString const hit = m_seedHit->text();
    if (hit.isEmpty()) return;

    int value = m_seedCalibrate->value();
    value += m_seedTime->value() - hit.toInt();
    m_seedCalibrate->setValue(value);
    m_seedHit->clear();
}

void RNGManipulation::OnUpdateContinueCalibrate()
{
    QString const hit = m_continueHit->text();
    if (hit.isEmpty()) return;

    int value = m_continueCalibrate->value();
    value += m_continueFrames->value() - hit.toInt();
    m_continueCalibrate->setValue(value);
    m_continueHit->clear();
}

}
