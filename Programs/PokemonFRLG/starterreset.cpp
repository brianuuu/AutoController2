#include "starterreset.h"

namespace Program::PokemonFRLG
{

StarterReset::StarterReset(QObject *parent)
    : ProgramBase{parent}
{
    
}

void StarterReset::PopulateSettings(QBoxLayout *layout)
{
    m_seedFrame = new Setting::SettingSpinBox("SeedFrame", 0, 9999);
    m_savedSettings.insert(m_seedFrame);
    AddSetting(layout, "Seed Frame:", "Wait this many frames (not accurate) before pressing A at the title screen. This setting is changed by the program while running", m_seedFrame, true);

    m_advanceFrame = new Setting::SettingSpinBox("AdvanceFrame", 0, 9999);
    m_savedSettings.insert(m_advanceFrame);
    AddSetting(layout, "Advance Frame:", "Wait this many frames (not accurate) before finishing dialogue after picking starter. This setting is changed by the program while running", m_advanceFrame, true);

    m_confirmDelay = new Setting::SettingSpinBox("ConfirmDelay", 0, 9999, 1000);
    m_savedSettings.insert(m_confirmDelay);
    AddSetting(layout, "Confirm Delay:", "Delay in ms between pressing yes and generating starter Pokemon before adding Advance Frame, you shouldn't have to change this", m_confirmDelay, true);

    AddSpacer(layout);
}

void StarterReset::RegisterStats()
{
    RegisterStat(m_statReset, "Resets");
    RegisterStat(m_statShiny, "Shinies");
}

void StarterReset::Start()
{
    ProgramBase::Start();

    m_currentMaxFrame = qMax(m_seedFrame->value(), m_advanceFrame->value());
    StateSoftReset();
}

void StarterReset::Stop()
{
    ProgramBase::Stop();
}

void StarterReset::StateSoftReset()
{
    PrintLog("Permutation: " + QString::number(m_seedFrame->value()) + "-" + QString::number(m_advanceFrame->value()), LOG_Important);
    m_state = SetState(State::SoftReset, "Restarting game");
    AddRunCommand("FRLG_SoftReset", 0);
    ++m_statReset;
}

void StarterReset::GetNextPermutation()
{
    // Order: 0-0, 1-0, 0-1, 2-0, 2-1, 2-2, 1-2, 0-2, 3-0, 3-1, 3-2, 3-3, 2-3, 1-3, 0-3, 4-0, etc.
    if (m_advanceFrame->value() == m_currentMaxFrame)
    {
        if (m_seedFrame->value() == 0)
        {
            ++m_currentMaxFrame;
            m_seedFrame->setValue(m_currentMaxFrame);
            m_advanceFrame->setValue(0);
        }
        else
        {
            m_seedFrame->setValue(m_seedFrame->value() - 1);
        }
    }
    else
    {
        m_advanceFrame->setValue(m_advanceFrame->value() + 1);
    }
}

void StarterReset::OnCommandFinished()
{
    if (OnModuleErrorQuit()) return;
    ClearModule(sender());

    switch (m_state)
    {
    case State::SoftReset:
    {
        m_state = SetState(State::TitleScreen, "Waiting for " + QString::number(m_seedFrame->value()) + " frames and enter title screen");
        AddRunCommand("FRLG_EnterGame", m_seedFrame->value() * 20);
        break;
    }
    case State::TitleScreen:
    {
        m_state = SetState(State::GetStarter, "Spam A until Yes/No box appears");
        AddRunCommand("A|Spam|10000");
        AddFrameCapture("FRLG_StarterYesNo");
        break;
    }
    case State::GetStarter:
    {
        // TODO: error stat?
        PrintLog("Unable to detect Yes/No box for too long", LOG_Error);
        emit notifyFinished(-1);
        break;
    }
    case State::ConfirmStarter:
    {
        m_state = SetState(State::MenuToStarter, "Go to starter's summary");
        AddRunCommand("FRLG_StarterCheck", 0);
        break;
    }
    case State::MenuToStarter:
    {
        m_state = SetState(State::CheckStarter, "Checking if starter is shiny");
        AddFrameCapture("FRLG_SummaryShiny");
        break;
    }
    case State::Capture:
    {
        // finished!
        emit notifyFinished(0);
        break;
    }
    default:
    {
        PrintLog("Unhandled state after command is finished", LOG_Error);
        emit notifyFinished(-1);
        return;
    }
    }
}

void StarterReset::OnFrameCaptureMatched(bool matched)
{
    if (OnModuleErrorQuit()) return;

    switch (m_state)
    {
    case State::GetStarter:
    {
        if (matched)
        {
            ClearModules();
            m_state = SetState(State::ConfirmStarter, "Waiting for " + QString::number(m_advanceFrame->value()) + " frames and confirming starter");
            AddRunCommand("None|" + QString::number(m_advanceFrame->value() * 20 + m_confirmDelay->value()) + ",B|Spam|11000");
        }
        break;
    }
    case State::CheckStarter:
    {
        ClearModules();
        if (matched)
        {
            PrintLog("Starter No." + m_statReset.GetString() + " is SHINY!", LOG_Success);
            SendDiscordMessage("Shiny Found!", true, false, true, LOG_Shiny);

            m_state = SetState(State::Capture, "Taking screenshot");
            AddRunCommand("Capture|50,Nothing|100");
            ++m_statShiny;
        }
        else
        {
            PrintLog("Starter No." + m_statReset.GetString() + " is not shiny...", LOG_Warning);
            GetNextPermutation();
            StateSoftReset();
        }
        break;
    }
    default:
    {
        PrintLog("Unhandled state after frame capture has result", LOG_Error);
        emit notifyFinished(-1);
        return;
    }
    }
}

}
