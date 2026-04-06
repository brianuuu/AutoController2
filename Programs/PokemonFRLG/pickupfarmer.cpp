#include "pickupfarmer.h"
#include "Modules/PokemonFRLG/blackscreen.h"

namespace Program::PokemonFRLG
{

void PickupFarmer::PopulateSettings(QBoxLayout *layout)
{
    m_maxPP = new Setting::SettingSpinBox("MaxPP", 5, 40);
    AddSetting(layout, "Max PP:", "Max PP for the first move of the first Pokemon, must be divisible by 5", m_maxPP);

    m_stopForShiny = new Setting::SettingCheckBox("StopForShiny", "", false);
    AddSetting(layout, "Stop For Shiny:", "Stop the program is a shiny is detected", m_stopForShiny);

    m_savedSettings.insert(m_maxPP);
    m_savedSettings.insert(m_stopForShiny);

    AddSpacer(layout);
}

void PickupFarmer::RegisterStats()
{
    RegisterStat(m_statEncounter, "Encounter");
    RegisterStat(m_statShiny, "Shiny");
}

void PickupFarmer::Start()
{
    ProgramBase::Start();

    // always heal at start
    m_battleCount = 9999;
    StateFetchItem();
}

void PickupFarmer::Stop()
{
    ProgramBase::Stop();
}

void PickupFarmer::OnCommandFinished(Module::Common::RunCommand* module)
{
	if (OnModuleErrorQuit(module)) return;
    m_moduleHolder->ClearModule(module);
	
	switch (m_state)
    {
    case State::Heal:
    {
        StateMove();
        break;
    }
    case State::BattleSelection:
    {
        emit notifyFinished(false, "Unable to detect battle selection for too long");
        break;
    }
    case State::BattleFinish:
    {
        emit notifyFinished(false, "Unable to detect battle finishing for too long");
        break;
    }
    case State::FetchItem:
    {
        Restart();
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

void PickupFarmer::OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched)
{
	if (OnModuleErrorQuit(module)) return;
	
	switch (m_state)
    {
    case State::BattleSelection:
    {
        if (matched)
        {
            qint64 const elapsed = m_elapsedTimer.elapsed();
            if (m_battleDelay == 0)
            {
                m_battleDelay = elapsed;
            }

            QString log = "Battle selection delay = " + QString::number(elapsed) + "ms";
            if (elapsed > m_battleDelay + 500)
            {
                PrintLog(log + " > " + QString::number(m_battleDelay) + "ms + 500ms", LOG_Success);
                ++m_statShiny;

                if (m_stopForShiny->isChecked())
                {
                    PrintLog("SHINY POKEMON FOUND!", LOG_Success);
                    m_moduleHolder->ClearModules();
                    m_state = SetState(State::Capture, "Capturing video");
                    m_moduleHolder->AddRunCommand("System_CaptureHome");
                    SendDiscordMessage("Shiny Found!", true, false, true, LOG_Shiny);
                    break;
                }
                else
                {
                    PrintLog("SHINY POKEMON FOUND! Ignoring according to setting...", LOG_Warning);
                    SendDiscordMessage("Shiny Found! (Ignored)", true, false, true, LOG_Shiny);
                }
            }
            else
            {
                PrintLog(log, LOG_Important);
            }

            m_moduleHolder->ClearModules();
            if (m_shouldRun)
            {
                m_state = SetState(State::BattleFinish, "Running away");
                m_moduleHolder->AddRunCommand("FRLG_RunFromEncounter");
                m_moduleHolder->AddFrameCapture("FRLG_CenterBlack");
            }
            else
            {
                m_battleCount++;
                m_state = SetState(State::BattleFinish, "Defeating Pokemon (PP Left: " + QString::number(m_maxPP->value() - m_battleCount) + ")");
                m_moduleHolder->AddRunCommand("A|Spam|500,B|Spam|10000");
                m_moduleHolder->AddFrameCapture("FRLG_CenterBlack");
            }
        }
        break;
    }
    case State::BattleFinish:
    {
        if (matched)
        {
            m_moduleHolder->ClearModules();
            m_timer.start(2000);
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

void PickupFarmer::OnSubModuleResult(Module::SubModuleBase *module, int result)
{
    if (OnModuleErrorQuit(module)) return;

    switch (m_state)
    {
    case State::Heal:
    {
        if (result == 0)
        {
            m_shouldRun = true;
            m_moduleHolder->ClearRunCommand();
            PrintLog("Encounter happened while trying to heal", LOG_Warning);
            m_state = SetState(State::EncounterStart);
            m_elapsedTimer.restart();
        }
        break;
    }
    case State::Move:
    {
        if (m_elapsedTimer.elapsed() > 60000)
        {
            emit notifyFinished(false, "Unable to detect encounter for too long");
        }
        else if (result == 0)
        {
            m_shouldRun = false;
            m_moduleHolder->ClearRunCommand();
            ++m_statEncounter;
            m_state = SetState(State::EncounterStart, "Encounter " + m_statEncounter.GetString() + " started");
            m_elapsedTimer.restart();
        }
        break;
    }
    case State::EncounterStart:
    {
        if (result == 1 && m_elapsedTimer.elapsed() > 500)
        {
            m_state = SetState(State::BattleSelection, "Waiting for Battle Selection");
            m_moduleHolder->ClearModules();
            m_moduleHolder->AddRunCommand("B|Spam|10000");
            m_moduleHolder->AddFrameCapture("FRLG_BattleBox");
            m_elapsedTimer.restart();
            if (m_battleDelay == 0)
            {
                PrintLog("Calibrating battle selection delay...");
            }
        }
        break;
    }
    default:
    {
        UnhandedStateSubModule();
        return;
    }
    }
}

void PickupFarmer::OnWaitTimeout()
{
    switch (m_state)
    {
    case State::BattleFinish:
    {
        if (m_battleCount % 5 == 0)
        {
            StateFetchItem();
        }
        else
        {
            Restart();
        }
        break;
    }
    default:
    {
        emit notifyFinished(false);
        return;
    }
    }
}

void PickupFarmer::Restart()
{
    if (m_battleCount >= m_maxPP->value())
    {
        StateHeal();
    }
    else
    {
        StateMove();
    }

    auto* module = new Module::PokemonFRLG::BlackScreen();
    connect(module, &Module::PokemonFRLG::BlackScreen::notifyResult, this, &PickupFarmer::OnSubModuleResult);
    m_moduleHolder->AddModule(module);
}

void PickupFarmer::StateHeal()
{
    m_battleCount = 0;
    m_state = SetState(State::Heal, "Healing Pokemon");
    m_moduleHolder->AddRunCommand("FRLG_PokemonTowerHeal");
}

void PickupFarmer::StateMove()
{
    m_state = SetState(State::Move, "Spinning in place until encounter");
    m_moduleHolder->AddRunCommand("FRLG_SpinInPlaceDown");
    m_elapsedTimer.restart();
}

void PickupFarmer::StateFetchItem()
{
    m_state = SetState(State::FetchItem, "Fetching items");
    m_moduleHolder->AddRunCommand("FRLG_FetchPickupItems");
}

}
