#include "respawnreset.h"
#include "Managers/audiomanager.h"

namespace Program::PokemonLZA
{

RespawnReset::RespawnReset(QObject *parent)
    : ProgramBase{parent}
{
    
}

void RespawnReset::PopulateSettings(QBoxLayout *layout)
{
    m_waitTime = new Setting::SettingDoubleSpinBox("WaitTime", 1.0, 30.0, 5.0);
    m_savedSettings.insert(m_waitTime);
    AddSetting(layout, "Wait Time:", "How many seconds to wait for shiny sound after game started", m_waitTime, true);

    AddSpacer(layout);
}

void RespawnReset::RegisterStats()
{
    RegisterStat(m_statReset, "Resets");
    RegisterStat(m_statShiny, "Shinies");
}

void RespawnReset::Start()
{
    ProgramBase::Start();

    m_shinySoundID = m_audioManager->AddDetection("PLA_Shiny");
    if (m_shinySoundID == 0)
    {
        emit notifyFinished(false);
        return;
    }

    m_state = SetState(State::Restart, "Restarting game");
    m_moduleHolder->AddRunCommand("System_RestartGame", 0);
    ++m_statReset;
}

void RespawnReset::Stop()
{
    ProgramBase::Stop();
}

void RespawnReset::OnCommandFinished()
{
    if (OnModuleErrorQuit()) return;
    m_moduleHolder->ClearModule(sender());

    switch (m_state)
    {
    case State::Restart:
    case State::GameLoadStart:
    {
        m_moduleHolder->AddFrameCapture("PLZA_LoadingBlackScreen");
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

void RespawnReset::OnFrameCaptureMatched(bool matched)
{
    if (!sender()) return;

    switch (m_state)
    {
    case State::Restart:
    case State::GameLoadStart:
    {
        // wait for black screen
        if (matched)
        {
            m_elapsedTimer.start();
            m_state = SetState((State)((int)m_state + 1));
        }
        break;
    }
    case State::TitleScreen:
    case State::GameLoadWait:
    {
        // wait for black screen to be not black anymore + buffer from black detection
        if (!matched && m_elapsedTimer.elapsed() > 300)
        {
            m_moduleHolder->ClearModule(sender());
            if (m_state == State::TitleScreen)
            {
                m_state = SetState(State::GameLoadStart, "Title screen detected, entering game");
                m_moduleHolder->AddRunCommand("A|Spam|2500");
            }
            else if (m_state == State::GameLoadWait)
            {
                m_state = SetState(State::Detect, "Listening for shiny sound for " + QString::number(m_waitTime->value()) + "s");
                m_audioManager->StartDetection(m_shinySoundID);
                m_timer.start(m_waitTime->value() * 1000);
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

void RespawnReset::OnWaitTimeout()
{
    m_state = SetState(State::Restart, "No shiny detected, restarting game");
    m_audioManager->StopDetection(m_shinySoundID);
    m_moduleHolder->AddRunCommand("System_RestartGame", 0);
    ++m_statReset;
}

void RespawnReset::OnSoundDetected(int id)
{
    m_timer.stop();
    PrintLog("SHINY POKEMON FOUND!", LOG_Success);

    m_state = SetState(State::Capture, "Capturing video");
    m_moduleHolder->AddRunCommand("PLZA_CaptureHome", 0);
    ++m_statShiny;

    SendDiscordMessage("Shiny Found!", true, false, true, LOG_Shiny);
}

}
