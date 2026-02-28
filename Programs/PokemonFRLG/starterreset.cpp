#include "starterreset.h"

namespace Program::PokemonFRLG
{

StarterReset::StarterReset(QObject *parent)
    : ProgramBase{parent}
{
    
}

void StarterReset::RegisterStats()
{
    RegisterStat(m_statReset, "Resets");
    RegisterStat(m_statShiny, "Shinies");
}

void StarterReset::Start()
{
    ProgramBase::Start();
    StateSoftReset();
}

void StarterReset::Stop()
{
    ProgramBase::Stop();
}

void StarterReset::StateSoftReset()
{
    m_state = SetState(State::SoftReset, "Restarting game");
    AddRunCommand("FRLG_SoftReset", 0);
    IncrementStat(m_statReset);
}

void StarterReset::OnCommandFinished()
{
    if (OnModuleErrorQuit()) return;
    ClearModule(sender());

    switch (m_state)
    {
    case State::SoftReset:
    {
        m_state = SetState(State::Save, "Saving to advance frames");
        AddRunCommand("FRLG_StarterSave", 0);
        break;
    }
    case State::Save:
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
            m_state = SetState(State::ConfirmStarter, "Confirming starter");
            AddRunCommand("B|Spam|12000");
        }
        break;
    }
    case State::CheckStarter:
    {
        ClearModules();
        if (matched)
        {
            PrintLog("Starter No." + QString::number(m_statReset) + " is SHINY!", LOG_Success);
            m_state = SetState(State::Capture, "Taking screenshot");
            AddRunCommand("Capture|50,Nothing|100");
        }
        else
        {
            PrintLog("Starter No." + QString::number(m_statReset) + " is not shiny...", LOG_Warning);
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
