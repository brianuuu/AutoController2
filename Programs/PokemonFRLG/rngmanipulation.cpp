#include "rngmanipulation.h"

namespace Program::PokemonFRLG
{

void RNGManipulation::PopulateSettings(QBoxLayout *layout)
{
    m_tid = new Setting::SettingSpinBox("TID", 0, 99999);
    m_savedSettings.insert(m_tid);
    AddSetting(layout, "Trainer ID:", "Saves your TID, does not affect the program", m_tid);

    m_sid = new Setting::SettingSpinBox("SID", 0, 99999);
    m_savedSettings.insert(m_sid);
    AddSetting(layout, "Secret ID:", "Saves your SID, does not affect the program\n(WARNING: Restoring Default Settings will clear this)", m_sid);

    m_seedTime = new Setting::SettingSpinBox("SeedTime", 29500, INT_MAX, 0);
    m_savedSettings.insert(m_seedTime);
    AddSetting(layout, "Seed Time:", "How many ms to wait from Home screen until pressing A at title screen", m_seedTime);

    m_seedCalibrate = new Setting::SettingSpinBox("SeedCalibrate", -10000, 10000, 0);
    m_savedSettings.insert(m_seedCalibrate);
    AddSetting(layout, "Seed Calibrate:", "How many ms off from your selected seed and hit seed (selected - hit)", m_seedCalibrate);

    m_continueFrames = new Setting::SettingSpinBox("ContinueFrames", 190, INT_MAX, 0);
    m_savedSettings.insert(m_continueFrames);
    AddSetting(layout, "Continue Screen Frames:", "How many frames to wait at continue screen before pressing A, 1 advance per frame", m_continueFrames);

    m_continueCalibrate = new Setting::SettingSpinBox("ContinueCalibrate", -10000, 10000, 0);
    m_savedSettings.insert(m_continueCalibrate);
    AddSetting(layout, "Continue Calibrate:", "How many frames off from your selected continue screen frames and hit frame (selected - hit)", m_continueCalibrate);

    m_overworldFrames = new Setting::SettingSpinBox("OverworldFrames", 230, INT_MAX, 600);
    m_savedSettings.insert(m_overworldFrames);
    AddSetting(layout, "Overworld Frames:", "How many frames to wait in the overworld before pressing A, 2 advances per frame", m_overworldFrames);

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
    return ProgramBase::CanRun() && m_commandFlashback->IsValid() && m_commandComplete->IsValid();
}

void RNGManipulation::Start()
{
    ProgramBase::Start();

    int const waitTime = m_seedTime->value() + m_seedCalibrate->value();
    m_timer.start(waitTime);

    m_state = SetState(State::TitleScreen, "Waiting for " + QString::number(waitTime) + "ms at Title Screen");
    AddRunCommand("A|100");
}

void RNGManipulation::Stop()
{
    ProgramBase::Stop();
}

void RNGManipulation::OnCommandFinished()
{
	if (OnModuleErrorQuit()) return;
    ClearModule(sender());
	
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
        AddRunCommand(m_commandFlashback->GetText());
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
        AddRunCommand("A|3000");
        break;
    }
    case State::ContinueScreen:
    {
        int const overworldFrames = m_overworldFrames->value();
        m_timer.start(overworldFrames * 1000 / 60);

        m_state = SetState(m_commandFlashback->GetText().isEmpty() ? State::CommandFlashback : State::Flashback, "Skipping flashback");
        AddRunCommand("A|Spam|200,B|Spam|3500");
        break;
    }
    case State::Flashback:
    {
        emit notifyFinished(false, "Overworld Frames is not enough after running user command");
        break;
    }
    case State::CommandFlashback:
    {
        m_state = SetState(State::CommandComplete, "Final A press and running user command if provided");
        QString const userCommand = m_commandComplete->GetText();
        AddRunCommand("A|50,None|50" + (userCommand.isEmpty() ? "" : "," + userCommand));
        break;
    }
    default:
    {
        emit notifyFinished(false);
        break;
    }
    }
}

}
