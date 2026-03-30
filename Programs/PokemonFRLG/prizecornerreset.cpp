#include "prizecornerreset.h"

namespace Program::PokemonFRLG
{

PrizeCornerReset::PrizeCornerReset(QObject *parent)
    : PermutationBase{parent}
{
    
}

void PrizeCornerReset::PopulateSettings(QBoxLayout *layout)
{
    m_slot = new Setting::SettingSpinBox("Slot", 1, 5);
    m_savedSettings.insert(m_slot);
    AddSetting(layout, "Prize Slot:", "Which slot of Pokemon to obtain", m_slot);

    m_count = new Setting::SettingSpinBox("Count", 1, 5);
    m_savedSettings.insert(m_count);
    AddSetting(layout, "Prize Count:", "How many of the same prize to get, you must have enough coins and empty party spaces", m_count);

    PermutationBase::PopulateSettings(layout);

    AddSpacer(layout);
}

void PrizeCornerReset::RegisterStats()
{
    PermutationBase::RegisterStats();
    RegisterStat(m_statPrize, "Prizes");
}

void PrizeCornerReset::Start()
{
    PermutationBase::Start();
    StateSoftReset();
}

void PrizeCornerReset::Stop()
{
    PermutationBase::Stop();
}

void PrizeCornerReset::StateSoftReset()
{
    PrintLog("Permutation: " + QString::number(m_seedFrame->value()) + "-" + QString::number(m_advanceFrame->value()), LOG_Important);
    m_state = SetState(State::SoftReset, "Restarting game");
    m_moduleHolder->AddRunCommand("FRLG_SoftReset", 0);
    ++m_statReset;
}

void PrizeCornerReset::StateWaitDialogue()
{
    m_dialogCount = 0;
    m_state = SetState(State::WaitDialogue, "Press A and spam B until all dialogue finishes");
    m_moduleHolder->AddRunCommand("(A|50,None|50)2,B|Spam|10000");
    m_moduleHolder->AddFrameCapture("FRLG_DialogBox");
}

void PrizeCornerReset::StateGetPrize()
{
    m_state = SetState(State::YesNoBox, "Getting prize No." + m_statPrize.GetString() + " at slot " + QString::number(m_slot->value()));
    QString command = "(A|900,None|100)2";
    if (m_slot->value() > 1)
    {
        command += ",(LDown|50,None|50)" + QString::number(m_slot->value() - 1);
    }
    command += ",A|Spam|10000";
    m_moduleHolder->AddRunCommand(command);
    m_moduleHolder->AddFrameCapture("FRLG_YesNoBox");
}

void PrizeCornerReset::OnCommandFinished(Module::Common::RunCommand* module)
{
    if (OnModuleErrorQuit(module)) return;
    m_moduleHolder->ClearModule(module);

    switch (m_state)
    {
    case State::SoftReset:
    {
        m_state = SetState(State::TitleScreen, "Waiting for " + QString::number(m_seedFrame->value()) + " frames and enter title screen");
        m_moduleHolder->AddRunCommand("FRLG_EnterGame", m_seedFrame->value() * 20);
        break;
    }
    case State::TitleScreen:
    {
        PrintLog("Waiting for " + QString::number(m_advanceFrame->value()) + " frames");
        m_timer.start(m_advanceFrame->value() * 20);
        break;
    }
    case State::YesNoBox:
    {
        emit notifyFinished(false, "Unable to detect yes/no box for too long");
        break;
    }
    case State::WaitDialogue:
    {
        emit notifyFinished(false, "Unable to detect dialogue finish for too long");
        break;
    }
    case State::PokemonSummary:
    {
        m_state = SetState(State::CheckShiny, "Checking if any prize is shiny");
        if (m_count->value() > 1)
        {
            m_moduleHolder->AddRunCommand("(LUp|100,None|500)" + QString::number(m_count->value() - 1) + ",None|1000");
        }
        else
        {
            m_moduleHolder->AddRunCommand("None|200");
        }
        m_moduleHolder->AddFrameCapture("FRLG_SummaryShiny");
        break;
    }
    case State::CheckShiny:
    {
        m_moduleHolder->ClearModules();
        PrintLog("No prizes are shiny...", LOG_Warning);
        GetNextPermutation();
        StateSoftReset();
        break;
    }
    case State::Capture:
    {
        // finished!
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

void PrizeCornerReset::OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched)
{
    if (OnModuleErrorQuit(module)) return;

    switch (m_state)
    {
    case State::YesNoBox:
    {
        if (matched)
        {
            m_moduleHolder->ClearModules();
            StateWaitDialogue();

            ++m_statPrize;
            ++m_currentCount;
        }
        break;
    }
    case State::WaitDialogue:
    {
        if (m_dialogCount == 0 && matched)
        {
            // dialogue started
            m_elapsedTimer.restart();
            ++m_dialogCount;
        }
        else if (m_dialogCount == 1 && !matched && m_elapsedTimer.elapsed() > 300)
        {
            // dialogue finished
            m_moduleHolder->ClearModules();
            if (m_currentCount < m_count->value())
            {
                StateGetPrize();
            }
            else
            {
                m_state = SetState(State::PokemonSummary, "Go to last Pokemon's summary");
                m_moduleHolder->AddRunCommand("FRLG_LastPartyCheck", 0);
            }
        }
        break;
    }
    case State::CheckShiny:
    {
        if (matched)
        {
            PrintLog("Gift No." + m_statReset.GetString() + " is SHINY!", LOG_Success);

            m_state = SetState(State::Capture, "Capturing video");
            m_moduleHolder->AddRunCommand("System_CaptureHome", 0);
            ++m_statShiny;

            SendDiscordMessage("Shiny Found!", true, false, true, LOG_Shiny);
        }
        break;
    }
    default:
    {
        UnhandedStateFrameCapture();
        return;
    }
    }
}

void PrizeCornerReset::OnWaitTimeout()
{
    // State::TitleScreen only
    m_currentCount = 0;
    StateGetPrize();
}

}
