#include "autobattlersf3.h"

namespace Program::MMSFLC
{

void AutoBattlerSF3::PopulateSettings(QBoxLayout *layout)
{
    m_count = new Setting::SettingSpinBox("BattleCount", 0, 999999, 1);
    AddSetting(layout, "Battle Count:", "How many battles to do, set 0 for infinite", m_count, true);
    m_savedSettings.insert(m_count);

    AddSpacer(layout);
}

void AutoBattlerSF3::RegisterStats()
{
    RegisterStat(m_statBattles, "Battles");
}

void AutoBattlerSF3::Start()
{
    ProgramBase::Start();

    m_top = Q_NULLPTR;
    m_bottom = Q_NULLPTR;
    m_currentCount = 0;
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
        emit notifyFinished(false, "Unable to detect battle custom screen for too long");
        break;
    }
    case State::UseDefaultCard:
    {
        emit notifyFinished(false, "Unable to detect battle complete for too long");
        break;
    }
    case State::CancelNoise:
    {
        emit notifyFinished(false, "Unable to detect noise change cancel dialog for too long");
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
                m_state = SetState(State::NoiseChange, "Noise Change detected");
                m_moduleHolder->ClearModules();
                m_moduleHolder->AddFrameCapture("MMSF3_TopDialog");
                m_elapsedTimer.restart();
            }
            else
            {
                StateEndBattle();
            }

            m_top = Q_NULLPTR;
            m_bottom = Q_NULLPTR;
        }
        break;
    }
    case State::NoiseChange:
    {
        if (m_elapsedTimer.elapsed() > 10000)
        {
            emit notifyFinished(false, "Unable to detect noise change dialog for too long");
        }
        else if (matched && m_elapsedTimer.elapsed() > 500)
        {
            m_state = SetState(State::CancelNoise, "Waiting for dialog to finish");
            m_moduleHolder->ClearModules();
            m_moduleHolder->AddFrameCapture("MMSF3_TopMugshot");
            m_moduleHolder->AddRunCommand("B|5000");
            m_elapsedTimer.restart();
        }
        break;
    }
    case State::CancelNoise:
    {
        if (!matched && m_elapsedTimer.elapsed() > 500)
        {
            m_state = SetState(State::UseDefaultCard, "Cancelling Noise Change");
            m_moduleHolder->ClearModules();
            m_top = Q_NULLPTR;
            m_bottom = m_moduleHolder->AddFrameCapture("MMSFLC_BottomBlack");
            m_moduleHolder->AddRunCommand("None|1000,DRight|50,A|Spam|6000");
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
    m_state = SetState(State::Encounter, "Starting Encounter no." + QString::number(++m_currentCount));
    m_moduleHolder->ClearModules();
    m_moduleHolder->AddRunCommand("MMSF3_Encounter");
    m_moduleHolder->AddFrameCapture("MMSF3_CustomOK");
}

void AutoBattlerSF3::StateEndBattle()
{
    if (m_count->value() > 0 && m_currentCount >= m_count->value())
    {
        emit notifyFinished(true);
        return;
    }

    m_state = SetState(State::EndBattle, "Battle ended");
    m_moduleHolder->ClearModules();
    m_moduleHolder->AddRunCommand("None|1500");
}

}
