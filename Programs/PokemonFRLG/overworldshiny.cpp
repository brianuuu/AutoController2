#include "overworldshiny.h"
#include "Managers/audiomanager.h"
#include "Modules/PokemonFRLG/blackscreen.h"

namespace Program::PokemonFRLG
{

void OverworldShiny::PopulateSettings(QBoxLayout *layout)
{
    m_type = new Setting::SettingComboBox("Type", {"Up/Down", "Left/Right", "Spin in Place"});
    AddSetting(layout, "Type:", "Move Up and Down, Left and Right, or Spin in Place", m_type, true);
    connect(m_type, &QComboBox::currentIndexChanged, this, &OverworldShiny::OnTypeChanged);

    m_moveTime = new Setting::SettingSpinBox("MoveTime", 100, 60000, 1000);
    AddSetting(layout, "Move Time:", "Move for this many milliseconds before turning around", m_moveTime, true);

    AddSpacer(layout);

    m_savedSettings.insert(m_type);
    m_savedSettings.insert(m_moveTime);
}

void OverworldShiny::RegisterStats()
{
    RegisterStat(m_statEncounter, "Encounter");
    RegisterStat(m_statShiny, "Shiny");
}

void OverworldShiny::Start()
{
    ProgramBase::Start();

    m_battleDelay = 0;
    m_shinySoundID = m_audioManager->AddDetection("FRLG_Shiny");
    if (m_shinySoundID == 0)
    {
        emit notifyFinished(false);
        return;
    }

    m_isUp = true;
    StateMove();
}

void OverworldShiny::Stop()
{
    ProgramBase::Stop();
}

void OverworldShiny::OnTypeChanged(int index)
{
    m_moveTime->setEnabled((Type)index != Type::SpinInPlace);
}

void OverworldShiny::OnCommandFinished(Module::Common::RunCommand* module)
{
    if (OnModuleErrorQuit(module)) return;
    m_moduleHolder->ClearModule(module);

    switch (m_state)
    {
    case State::Listen:
    {
        emit notifyFinished(false, "Unable to detect battle selection for too long");
        break;
    }
    case State::RunAway:
    {
        StateMove();
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

void OverworldShiny::OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched)
{
    if (OnModuleErrorQuit(module)) return;

    switch (m_state)
    {
    case State::Listen:
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
                OnSoundDetected(m_shinySoundID);
                break;
            }
            else
            {
                PrintLog(log, LOG_Important);
            }

            m_moduleHolder->ClearModules();
            m_state = SetState(State::RunAway, "No shiny detected, running away");
            m_audioManager->StopDetection(m_shinySoundID);
            m_moduleHolder->AddRunCommand("FRLG_RunFromEncounter", 0);
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

void OverworldShiny::OnSubModuleResult(Module::SubModuleBase *module, int result)
{
    if (OnModuleErrorQuit(module)) return;

    switch (m_state)
    {
    case State::Move:
    {
        if (m_elapsedTimer.elapsed() > 60000)
        {
            emit notifyFinished(false, "Unable to detect encounter for too long");
        }
        else if (result == 0)
        {
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
            m_moduleHolder->ClearModules();
            m_state = SetState(State::EncounterWait, "Wait 1 second");
            m_timer.start(1000);
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

void OverworldShiny::OnWaitTimeout()
{
    switch (m_state)
    {
    case State::EncounterWait:
    {
        m_state = SetState(State::Listen, "Listening for shiny sound");
        m_audioManager->StartDetection(m_shinySoundID);
        m_moduleHolder->AddRunCommand("B|Spam|10000");
        m_moduleHolder->AddFrameCapture("FRLG_BattleBox");

        m_elapsedTimer.restart();
        if (m_battleDelay == 0)
        {
            PrintLog("Calibrating battle selection delay...");
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

void OverworldShiny::OnSoundDetected(int id)
{
    PrintLog("SHINY POKEMON FOUND!", LOG_Success);

    m_moduleHolder->ClearModules();
    m_state = SetState(State::Capture, "Capturing video");
    m_moduleHolder->AddRunCommand("System_CaptureHome", 0);
    ++m_statShiny;

    SendDiscordMessage("Shiny Found!", true, false, true, LOG_Shiny);
}

void OverworldShiny::StateMove()
{
    m_state = SetState(State::Move, "Moving back and forth until encounter");

    QString const moveTime = QString::number(m_moveTime->value());
    switch ((Type)m_type->currentIndex())
    {
    case Type::UpDown:
        m_moduleHolder->AddRunCommand("(B|LUp|" + moveTime + ",B|LDown|" + moveTime + ")0");
        break;
    case Type::LeftRight:
        m_moduleHolder->AddRunCommand("(B|LLeft|" + moveTime + ",B|LRight|" + moveTime + ")0");
        break;
    case Type::SpinInPlace:
        m_moduleHolder->AddRunCommand(m_isUp ? "FRLG_SpinInPlaceUp" : "FRLG_SpinInPlaceDown", 0);
        m_isUp = !m_isUp;
        break;
    }

    auto* module = new Module::PokemonFRLG::BlackScreen();
    connect(module, &Module::PokemonFRLG::BlackScreen::notifyResult, this, &OverworldShiny::OnSubModuleResult);
    m_moduleHolder->AddModule(module);
    m_elapsedTimer.restart();
}

}
