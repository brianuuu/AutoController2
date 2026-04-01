#include "staticreset.h"
#include "Managers/audiomanager.h"
#include "Modules/PokemonFRLG/blackscreen.h"

namespace Program::PokemonFRLG
{

StaticReset::StaticReset(QObject *parent)
    : PermutationBase{parent}
{
    
}

void StaticReset::PopulateSettings(QBoxLayout *layout)
{
    m_moveUp = new Setting::SettingCheckBox("MoveUp");
    m_savedSettings.insert(m_moveUp);
    AddSetting(layout, "Move Up instead of A Spam:", "Replaces A spam with Up move, for Ho-Oh only", m_moveUp);

    PermutationBase::PopulateSettings(layout);

    AddSpacer(layout);
}

void StaticReset::Start()
{
    PermutationBase::Start();

    m_battleDelay = 0;
    m_shinySoundID = m_audioManager->AddDetection("FRLG_Shiny");
    if (m_shinySoundID == 0)
    {
        emit notifyFinished(false);
        return;
    }

    StateSoftReset();
}

void StaticReset::Stop()
{
    PermutationBase::Stop();
}

void StaticReset::StateSoftReset()
{
    PrintLog("Permutation: " + QString::number(m_seedFrame->value()) + "-" + QString::number(m_advanceFrame->value()), LOG_Important);
    m_state = SetState(State::SoftReset, "Restarting game");
    m_moduleHolder->AddRunCommand("FRLG_SoftReset", 0);
    ++m_statReset;
}

void StaticReset::OnCommandFinished(Module::Common::RunCommand* module)
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
        m_state = SetState(State::EncounterTrigger, "Waiting for " + QString::number(m_advanceFrame->value()) + " frames and triggering encounter");
        QString command = m_moveUp->isChecked() ? "LUp|10000" : "A|Spam|30000";
        if (m_advanceFrame->value() > 0)
        {
            command = "None|" + QString::number(m_advanceFrame->value() * 20) + "," + command;
        }
        m_moduleHolder->AddRunCommand(command);

        auto* module = new Module::PokemonFRLG::BlackScreen();
        connect(module, &Module::PokemonFRLG::BlackScreen::notifyResult, this, &StaticReset::OnSubModuleResult);
        m_moduleHolder->AddModule(module);
        m_elapsedTimer.restart();
        break;
    }
    case State::EncounterStart:
    {
        emit notifyFinished(false, "Unable to detect encounter for too long");
        break;
    }
    case State::Listen:
    {
        emit notifyFinished(false, "Unable to detect battle selection for too long");
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

void StaticReset::OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched)
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

            PrintLog("No shiny detected");
            m_moduleHolder->ClearModules();
            m_audioManager->StopDetection(m_shinySoundID);
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

void StaticReset::OnSubModuleResult(Module::SubModuleBase *module, int result)
{
    if (OnModuleErrorQuit(module)) return;

    switch (m_state)
    {
    case State::EncounterTrigger:
    {
        if (result == 0)
        {
            m_moduleHolder->ClearRunCommand();
            m_state = SetState(State::EncounterStart, "Encounter started");
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

void StaticReset::OnWaitTimeout()
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

void StaticReset::OnSoundDetected(int id)
{
    PrintLog("SHINY POKEMON FOUND!", LOG_Success);

    m_moduleHolder->ClearModules();
    m_state = SetState(State::Capture, "Capturing video");
    m_moduleHolder->AddRunCommand("System_CaptureHome", 0);
    ++m_statShiny;

    SendDiscordMessage("Shiny Found!", true, false, true, LOG_Shiny);
}

}
