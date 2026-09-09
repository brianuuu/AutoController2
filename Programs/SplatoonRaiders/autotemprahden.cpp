#include "autotemprahden.h"

namespace Program::Raiders
{

void AutoTemprahDen::PopulateSettings(QBoxLayout *layout)
{
    m_count = new Setting::SettingSpinBox("Count", 0, 99999);
    m_savedSettings.insert(m_count);
    AddSetting(layout, "Raid Count:", "No. of the raids to do (set 0 for infinite)", m_count, true);

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
            m_moduleHolder->AddRunCommand("Raiders_AutoFire");

            // detections
            m_blackDetected = false;
            m_fail = m_moduleHolder->AddFrameCapture("System_CenterBlack", QColor(255,0,0));

            // raid is 90s long, set limit to 120s
            m_timer.start(12000);

            ++m_raidCount;
            ++m_statRaids;
        }
        break;
    }
    case State::AutoFire:
    {
        if (matched)
        {
            if (module == m_fail)
            {
                m_elapsedTimer.restart();
                m_timer.stop();
                m_blackDetected = true;

                m_state = SetState(State::FinishRaid, "Raid failed...");
                m_moduleHolder->ClearModules();
                m_moduleHolder->AddFrameCapture("System_CenterBlack");
            }
            else
            {
                // TODO:
            }

            m_fail = Q_NULLPTR;
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
    PrintLog("Unable to detect raid completion for too long", LOG_Error);
    m_state = SetState(State::FinishRaid, "Quitting raid");
    m_moduleHolder->ClearModules();
    m_moduleHolder->AddRunCommand("Raiders_QuitRaid");
    m_moduleHolder->AddFrameCapture("System_CenterBlack");

    m_fail = Q_NULLPTR;
}

void AutoTemprahDen::StateStartRaid()
{
    m_state = SetState(State::StartRaid, "Starting raid no." + QString::number(m_raidCount + 1));
    m_moduleHolder->ClearModules();
    m_moduleHolder->AddRunCommand("Raiders_StartRaid");
}

void AutoTemprahDen::StateFinishRaid(bool failed)
{
    m_elapsedTimer.restart();
    m_timer.stop();

    if (failed)
    {
        m_blackDetected = true;
        PrintLog("Raid failed...", LOG_Warning);
    }
    else
    {
        m_blackDetected = false;
        PrintLog("Raid successful!", LOG_Success);
        ++m_statSuccess;
    }

    m_state = SetState(State::FinishRaid);
    m_moduleHolder->ClearModules();
    m_moduleHolder->AddFrameCapture("System_CenterBlack");

    m_fail = Q_NULLPTR;
}

}
