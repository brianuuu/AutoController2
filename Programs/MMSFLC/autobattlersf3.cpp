#include "autobattlersf3.h"

namespace Program::MMSFLC
{

void AutoBattlerSF3::PopulateSettings(QBoxLayout *layout)
{
    AddSpacer(layout);
}

void AutoBattlerSF3::RegisterStats()
{
    RegisterStat(m_statBattles, "Battles");
}

void AutoBattlerSF3::Start()
{
    ProgramBase::Start();

    m_count = 0;
    StateStart();
}

void AutoBattlerSF3::Stop()
{
    ProgramBase::Stop();
}

void AutoBattlerSF3::OnCommandFinished(Module::Common::RunCommand* module)
{
	if (OnModuleErrorQuit(module)) return;
    m_moduleHolder->ClearModule(module);
	
	switch (m_state)
    {
    case State::Encounter:
    {
        emit notifyFinished(false, "Unable to battle custom screen for too long");
        break;
    }
    case State::UseDefaultCard:
    {
        emit notifyFinished(false, "Unable to battle complete for too long");
        break;
    }
    case State::EndBattle:
    {
        StateStart();
        break;
    }
    default:
    {
        UnhandedStateRunCommand();
        return;
    }
    }
}

void AutoBattlerSF3::OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched)
{
	if (OnModuleErrorQuit(module)) return;
	
	switch (m_state)
    {
    case State::Encounter:
    {
        if (matched)
        {
            m_state = SetState(State::UseDefaultCard, "Selecting and using default card");
            m_moduleHolder->ClearModules();
            m_moduleHolder->AddRunCommand("MMSF3_UseDefaultCard", 1000);

            m_top = m_moduleHolder->AddFrameCapture("MMSFLC_TopWhite");
            m_bottom = m_moduleHolder->AddFrameCapture("MMSFLC_BottomBlack");

            ++m_statBattles;
        }
        break;
    }
    case State::UseDefaultCard:
    {
        if (matched)
        {
            if (module == m_top)
            {
                //m_state = SetState(State::NoiseChange, "Selecting and using default card");
                emit notifyFinished(false, "Noise Change detected");
            }
            else
            {
                StateEndBattle();
            }
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

void AutoBattlerSF3::StateStart()
{
    m_state = SetState(State::Encounter, "Starting Encounter no." + QString::number(++m_count));
    m_moduleHolder->ClearModules();
    m_moduleHolder->AddRunCommand("MMSF3_Encounter");
    m_moduleHolder->AddFrameCapture("MMSF3_CustomOK");
}

void AutoBattlerSF3::StateEndBattle()
{
    m_state = SetState(State::EndBattle, "Battle ended");
    m_moduleHolder->ClearModules();
    m_moduleHolder->AddRunCommand("None|1500");
}

}
