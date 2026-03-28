#include "nuggetfarmer.h"

namespace Program::PokemonFRLG
{

void NuggetFarmer::PopulateSettings(QBoxLayout *layout)
{
    m_count = new Setting::SettingSpinBox("LoopCount", 1, 999, 1);
    AddSetting(layout, "Loop Count:", "How many loops to perform", m_count, true);
    m_savedSettings.insert(m_count);

    AddSpacer(layout);
}

void NuggetFarmer::RegisterStats()
{
    RegisterStat(m_statNuggets, "Nuggets");
}

void NuggetFarmer::Start()
{
    ProgramBase::Start();

    m_currentCount = 0;
    StateToNuggetBridge();
}

void NuggetFarmer::Stop()
{
    ProgramBase::Stop();
}

void NuggetFarmer::OnCommandFinished()
{
	if (OnModuleErrorQuit()) return;
    ClearModule(sender());
	
	switch (m_state)
    {
    case State::ToNuggetBridge:
    {
        emit notifyFinished(false, "Cannot detect dialog box with Team Rocket grunt");
        break;
    }
    case State::ReturnToPC:
    {
        emit notifyFinished(false, "Cannot detect finishing dialogue with Nurse Joy");
        break;
    }
    default:
    {
        UnhandedStateRunCommand();
        return;
    }
    }
}

void NuggetFarmer::OnFrameCaptureMatched(bool matched)
{
	if (OnModuleErrorQuit()) return;
	
	switch (m_state)
    {
    case State::ToNuggetBridge:
    {
        if (m_dialogCount == 0 && matched)
        {
            // dialogue started
            PrintLog("Dialogue started with Team Rocket grunt");
            m_elapsedTimer.restart();
            m_dialogCount++;
            AddRunCommand("(A|Spam|10000)0");
        }
        else if (m_dialogCount == 1 && !matched && m_elapsedTimer.elapsed() > 300)
        {
            // dialogue finished
            ClearModule(sender());
            m_elapsedTimer.restart();

            m_state = SetState(State::BattleBox, "Detecting black screen for battle start");
            AddFrameCapture("FRLG_EncounterBottom");
        }
        break;
    }
    case State::BattleBox:
    {
        if (m_elapsedTimer.elapsed() > 5000)
        {
            emit notifyFinished(false, "Unable to detect battle start for too long, did you maxed out nuggets?");
            return;
        }

        if (matched)
        {
            ClearModule(sender());
            m_elapsedTimer.restart();

            ++m_statNuggets;
            m_state = SetState(State::BattleLose, "Battle " + QString::number(m_currentCount + 1) + " started, spamming A until returning to PC");
            AddFrameCapture("FRLG_DialogBox");
        }
        break;
    }
    case State::BattleLose:
    {
        if (m_elapsedTimer.elapsed() > 120000)
        {
            emit notifyFinished(false, "Unable to detect returning to PC for too long");
            return;
        }

        if (matched)
        {
            // dialogue started
            m_elapsedTimer.restart();
            m_dialogCount++;

            m_state = SetState(State::ReturnToPC, "Returned to PC, spamming B to finish dialogue");
            AddRunCommand("B|Spam|10000");
        }
        break;
    }
    case State::ReturnToPC:
    {
        if (!matched && m_elapsedTimer.elapsed() > 300)
        {
            // dialogue finished
            ClearModules();
            if (++m_currentCount == m_count->value())
            {
                emit notifyFinished(true);
            }
            else
            {
                StateToNuggetBridge();
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

void NuggetFarmer::StateToNuggetBridge()
{
    PrintLog("Loop " + QString::number(m_currentCount + 1), LOG_Important);
    m_state = SetState(State::ToNuggetBridge, "Heading to Nugget Bridge");
    AddRunCommand("FRLG_ToNuggetBridge", 0);
    AddFrameCapture("FRLG_DialogBox");
    m_dialogCount = 0;
}

}
