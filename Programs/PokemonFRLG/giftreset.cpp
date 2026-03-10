#include "giftreset.h"

namespace Program::PokemonFRLG
{

GiftReset::GiftReset(QObject *parent)
    : PermutationBase{parent}
{
    
}

void GiftReset::PopulateSettings(QBoxLayout *layout)
{
    PermutationBase::PopulateSettings(layout);

    m_accept = new Setting::SettingCheckBox("Accept", "", true);
    m_savedSettings.insert(m_accept);
    AddSetting(layout, "Accept Gift:", "Check this if gift Pokemon requires pressing Yes (Eevee doesn't need this)", m_accept, true);

    AddSpacer(layout);
}

void GiftReset::Start()
{
    PermutationBase::Start();
    StateSoftReset();
}

void GiftReset::Stop()
{
    PermutationBase::Stop();
}

void GiftReset::StateSoftReset()
{
    PrintLog("Permutation: " + QString::number(m_seedFrame->value()) + "-" + QString::number(m_advanceFrame->value()), LOG_Important);
    m_state = SetState(State::SoftReset, "Restarting game");
    AddRunCommand("FRLG_SoftReset", 0);
    ++m_statReset;
    m_dialogCount = 0;
}

void GiftReset::StateWaitDialogue()
{
    m_state = SetState(State::WaitDialogue, "Press A and spam B until all dialogue finishes");
    QString command = "(A|50,None|50)2,B|Spam|10000";
    if (m_advanceFrame->value() > 0)
    {
        command = "None|" + QString::number(m_advanceFrame->value() * 20) + "," + command;
    }
    AddRunCommand(command);
    AddFrameCapture("FRLG_DialogBox");
}

void GiftReset::OnCommandFinished()
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
        PrintLog("Waiting for " + QString::number(m_advanceFrame->value()) + " frames");

        if (m_accept->isChecked())
        {
            m_state = SetState(State::YesNoBox, "Spam A until Yes/No box appears");
            QString command = "A|Spam|10000";
            if (m_advanceFrame->value() > 0)
            {
                command = "None|" + QString::number(m_advanceFrame->value() * 20) + "," + command;
            }
            AddRunCommand(command);
            AddFrameCapture("FRLG_YesNoBox");
        }
        else
        {
            StateWaitDialogue();
        }
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
    case State::CheckPokemon:
    {
        AddFrameCapture("FRLG_SummaryShiny");
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

void GiftReset::OnFrameCaptureMatched(bool matched)
{
    if (OnModuleErrorQuit()) return;

    switch (m_state)
    {
    case State::YesNoBox:
    {
        if (matched)
        {
            ClearModules();
            StateWaitDialogue();
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
            ClearModules();
            m_state = SetState(State::CheckPokemon, "Go to gift Pokemon's summary and check if it's shiny");
            AddRunCommand("FRLG_LastPartyCheck", 0);
        }
        break;
    }
    case State::CheckPokemon:
    {
        ClearModules();
        if (matched)
        {
            PrintLog("Gift No." + m_statReset.GetString() + " is SHINY!", LOG_Success);

            m_state = SetState(State::Capture, "Taking screenshot");
            AddRunCommand("Capture|50,Nothing|100");
            ++m_statShiny;

            SendDiscordMessage("Shiny Found!", true, false, true, LOG_Shiny);
        }
        else
        {
            PrintLog("Gift No." + m_statReset.GetString() + " is not shiny...", LOG_Warning);
            GetNextPermutation();
            StateSoftReset();
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

}
