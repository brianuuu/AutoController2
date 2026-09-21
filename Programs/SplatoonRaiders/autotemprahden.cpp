#include "autotemprahden.h"

namespace Program::Raiders
{

void AutoTemprahDen::PopulateSettings(QBoxLayout *layout)
{
    m_count = new Setting::SettingSpinBox("Count", 0, 99999);
    m_savedSettings.insert(m_count);
    AddSetting(layout, "Raid Count:", "No. of the raids to do (set 0 for infinite)", m_count);

    m_mash = new Setting::SettingCheckBox("Mash", "", false);
    m_savedSettings.insert(m_mash);
    AddSetting(layout, "Mash Fire:", "Mash ZR to fire weapon instead of hold", m_mash);

    AddSpacer(layout);
}

void AutoTemprahDen::RegisterStats()
{
    RegisterStat(m_statRaids, "Raids");
    RegisterStat(m_statSuccess, "Success");
}

void AutoTemprahDen::Start()
{
    ProgramBase::Start();

    m_fail = Q_NULLPTR;
    m_raidCount = 0;
    m_blackDetected = false;
    m_powerEggDetected = false;

    StateStartRaid();
}

void AutoTemprahDen::Stop()
{
    ProgramBase::Stop();
}

void AutoTemprahDen::OnCommandFinished(Module::Common::RunCommand* module)
{
	if (OnModuleErrorQuit(module)) return;
    m_moduleHolder->ClearModule(module);
	
	switch (m_state)
    {
    case State::StartRaid:
    {
        m_elapsedTimer.restart();
        m_moduleHolder->AddFrameCapture("Raiders_Health", QColor(255,0,0));
        break;
    }
    case State::CollectTreasure:
    {
        StateQuitRaid();
        break;
    }
    case State::FinishRaid:
    {
        // nothing, detect black screen
        break;
    }
    case State::CollectLoot:
    {
        emit notifyFinished(false, "Unable to detect health bar at outpost");
        break;
    }
    default:
    {
        UnhandedStateRunCommand();
        return;
    }
    }
}

void AutoTemprahDen::OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched)
{
	if (OnModuleErrorQuit(module)) return;
	
	switch (m_state)
    {
    case State::StartRaid:
    {
        if (m_elapsedTimer.elapsed() > 5000)
        {
            emit notifyFinished(false, "Unable to detect raid start for too long");
        }
        else if (matched)
        {
            m_state = SetState(State::AutoFire, "Raid started, auto firing weapon and all gadgets");
            m_moduleHolder->ClearModules();
            m_moduleHolder->AddRunCommand(m_mash->isChecked() ? "Raiders_AutoFireMash" : "Raiders_AutoFireHold");

            // detections
            m_blackDetected = false;
            m_powerEggDetected = false;
            m_fail = m_moduleHolder->AddFrameCapture("System_CenterBlack", QColor(255,0,0));
            m_moduleHolder->AddFrameCapture("Raiders_PowerEggGauge");
            // TODO: detect death

            // raid is 90s long, set limit to 120s
            m_timer.start(120000);

            ++m_raidCount;
            ++m_statRaids;
        }
        break;
    }
    case State::AutoFire:
    {
        if (module == m_fail)
        {
            if (matched)
            {
                StateFinishRaid(false);
            }
        }
        else
        {
            if (matched && !m_powerEggDetected)
            {
                m_elapsedTimer.restart();
                m_powerEggDetected = true;
            }
            else if (!matched && m_powerEggDetected && m_elapsedTimer.elapsed() > 10000)
            {
                m_timer.stop();

                m_state = SetState(State::CollectTreasure, "Attempting to collect treasure");
                m_moduleHolder->ClearModules();
                m_moduleHolder->AddFrameCapture("System_CenterBlack");
                m_moduleHolder->AddRunCommand("None|5000,ZL|LUp|15000");

                m_fail = Q_NULLPTR;
            }
        }
        break;
    }
    case State::CollectTreasure:
    {
        if (matched)
        {
            StateFinishRaid(true);
        }
        break;
    }
    case State::FinishRaid:
    {
        if (!m_blackDetected && matched)
        {
            m_elapsedTimer.restart();
            m_blackDetected = true;
        }
        else if (m_blackDetected && !matched && m_elapsedTimer.elapsed() > 300)
        {
            // wait for black screen to go away
            m_state = SetState(State::CollectLoot, "Collecting loots");
            m_moduleHolder->ClearModules();
            m_moduleHolder->AddRunCommand("A|Spam|10000");
            m_moduleHolder->AddFrameCapture("Raiders_Health", QColor(255,0,0));
        }
        break;
    }
    case State::CollectLoot:
    {
        if (matched)
        {
            if (StopNextCycle())
            {
                break;
            }

            if (m_raidCount == m_count->value())
            {
                emit notifyFinished(true);
            }
            else
            {
                StateStartRaid();
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

void AutoTemprahDen::OnWaitTimeout()
{
    // State::AutoFire only
    StateQuitRaid();
}

void AutoTemprahDen::StateStartRaid()
{
    m_state = SetState(State::StartRaid, "Starting raid no." + QString::number(m_raidCount + 1));
    m_moduleHolder->ClearModules();
    m_moduleHolder->AddRunCommand("Raiders_StartRaid");
}

void AutoTemprahDen::StateQuitRaid()
{
    PrintLog("Unable to detect raid completion for too long", LOG_Error);
    m_state = SetState(State::FinishRaid, "Quitting raid");
    m_moduleHolder->ClearModules();
    m_moduleHolder->AddRunCommand("Raiders_QuitRaid");
    m_moduleHolder->AddFrameCapture("System_CenterBlack");

    m_fail = Q_NULLPTR;
}

void AutoTemprahDen::StateFinishRaid(bool success)
{
    m_elapsedTimer.restart();
    m_timer.stop();

    if (success)
    {
        m_blackDetected = false;
        PrintLog("Raid successful!", LOG_Success);
        ++m_statSuccess;
    }
    else
    {
        m_blackDetected = true;
        PrintLog("Raid failed...", LOG_Warning);
    }

    m_state = SetState(State::FinishRaid);
    m_moduleHolder->ClearModules();
    m_moduleHolder->AddFrameCapture("System_CenterBlack");

    m_fail = Q_NULLPTR;
}

}
