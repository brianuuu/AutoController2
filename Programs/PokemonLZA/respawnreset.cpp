#include "respawnreset.h"
#include "Managers/audiomanager.h"
#include "Modules/Common/framecapture.h"
#include "Modules/Common/runcommand.h"

namespace Program::PokemonLZA
{

#define RESPAWN_RESET_RUN_COMMAND(...) AddModule<Module::Common::RunCommand>(&RespawnReset::OnCommandFinished, __VA_ARGS__)

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

    m_shinySoundID = m_audioManager->AddDetection("PokemonLA/ShinySFX", 0.19f, 5000);
    if (m_shinySoundID == 0)
    {
        emit notifyFinished(-1);
        return;
    }

    m_state = SetState(State::Restart, "Restarting game");
    RESPAWN_RESET_RUN_COMMAND("System_RestartGame", 0);
    IncrementStat(m_statReset);
}

void RespawnReset::Stop()
{
    ProgramBase::Stop();
}

void RespawnReset::OnCommandFinished()
{
    if (OnModuleErrorQuit()) return;
    ClearModule(sender());

    switch (m_state)
    {
    case State::Restart:
    case State::GameLoadStart:
    {
        Module::Common::FrameCapture* module = new Module::Common::FrameCapture("PLZA_LoadingBlackScreen");
        connect(module, &QThread::finished, this, &ProgramBase::OnModuleErrorQuit);
        connect(module, &Module::Common::FrameCapture::notifyResultMatched, this, &RespawnReset::OnFrameCaptureMatched);
        AddModule(module);
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
            ClearModule(sender());
            if (m_state == State::TitleScreen)
            {
                m_state = SetState(State::GameLoadStart, "Title screen detected, entering game");
                RESPAWN_RESET_RUN_COMMAND("A|Spam|2500");
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
        PrintLog("Unhandled state after frame capture has result", LOG_Error);
        emit notifyFinished(-1);
        return;
    }
    }
}

void RespawnReset::OnWaitTimeout()
{
    m_state = SetState(State::Restart, "No shiny detected, restarting game");
    m_audioManager->StopDetection(m_shinySoundID);
    RESPAWN_RESET_RUN_COMMAND("System_RestartGame", 0);
    IncrementStat(m_statReset);
}

void RespawnReset::OnSoundDetected(int id)
{
    m_timer.stop();

    // TODO: discord

    m_state = SetState(State::Capture, "Capturing video");
    RESPAWN_RESET_RUN_COMMAND("PLZA_CaptureHome", 0);
    PrintLog("SHINY POKEMON FOUND!", LOG_Success);
    IncrementStat(m_statShiny);
}

}
