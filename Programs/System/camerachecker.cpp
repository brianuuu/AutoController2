#include "camerachecker.h"
#include "Managers/profilemanager.h"

namespace Program::System
{

CameraChecker::CameraChecker(QObject *parent)
    : ProgramBase{parent}
{
    
}

void CameraChecker::Start()
{
    ProgramBase::Start();

    m_button = 0;
    m_delay = 0;

    m_state = SetState(State::DetectTheme, "Checking Nintendo Switch type and theme");
    m_moduleHolder->AddFrameCapture("System_HomeTheme");
}

void CameraChecker::Stop()
{
    ProgramBase::Stop();
}

void CameraChecker::OnCommandFinished(Module::Common::RunCommand* module)
{
    if (OnModuleErrorQuit(module)) return;
    m_moduleHolder->ClearModule(module);

    switch (m_state)
    {
    case State::SystemSetting:
    {
        m_state = SetState(State::ButtonMenu, "Go to button test menu");
        m_moduleHolder->AddRunCommand("System_ButtonTest", 0);
        break;
    }
    case State::ButtonMenu:
    {
        m_state = SetState(State::ButtonTest, "Testing camera delay");
        StateButtonTest();
        break;
    }
    case State::ButtonTest:
    {
        // nothing
        break;
    }
    case State::ReturnHome:
    {
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

void CameraChecker::OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched)
{
    if (OnModuleErrorQuit(module)) return;;

    switch (m_state)
    {
    case State::DetectTheme:
    {
        m_color = module->GetResultColor();
        m_moduleHolder->ClearModules();

        bool matchSystemType = true;
        if (CaptureHolder::GetColorMatch(m_color, ThemeLight))
        {
            PrintLog("Light Theme detected (Make sure System type in Global Settings is set correctly)", LOG_Important);
        }
        else if (CaptureHolder::GetColorMatch(m_color, ThemeDark1))
        {
            PrintLog("Nintendo Switch 1 Dark Theme detected", LOG_Important);
            matchSystemType = m_profileManager->GetSystemType() == ST_Swtich1;
        }
        else if (CaptureHolder::GetColorMatch(m_color, ThemeDark2))
        {
            PrintLog("Nintendo Switch 2 Dark Theme detected", LOG_Important);
            matchSystemType = m_profileManager->GetSystemType() == ST_Swtich2;
        }
        else
        {
            PrintLog("Unable to determine Nintendo Switch theme from color " + m_color.name() + ", display color from capture card maybe inaccurate. "
                "(Is HDR option turned off for Nintendo Switch 2? Both Console Screen’s HDR Output AND HDR Output setting below)", LOG_Warning);
        }

        if (!matchSystemType)
        {
            emit notifyFinished(false, "Incorrect System type set in Global Settings");
        }
        else
        {
            m_state = SetState(State::SystemSetting, "Go to system settings");
            m_moduleHolder->AddRunCommand("System_Settings", 0);
        }
        break;
    }
    case State::ButtonTest:
    {
        QColor const color = module->GetResultColor();
        if (!CaptureHolder::GetColorMatch(color, m_color))
        {
            m_moduleHolder->ClearModules();

            qint64 const elapsed = m_elapsedTimer.elapsed();
            m_delay += elapsed;
            PrintLog("Delay = " + QString::number(elapsed) + "ms");

            if (m_button == 4)
            {
                m_delay /= 4;
                PrintLog("Average Delay = " + QString::number(m_delay) + "ms", LOG_Important);
                if (m_delay >= 1000)
                {
                    PrintLog("Over 1 second delay may cause programs to not work properly", LOG_Warning);
                }

                m_state = SetState(State::ReturnHome);
                m_moduleHolder->AddRunCommand("Home|50,None|100");
            }
            else
            {
                StateButtonTest();
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

void CameraChecker::StateButtonTest()
{
    QString prefix;
    switch (m_profileManager->GetSystemType())
    {
    case ST_Swtich1: prefix = "System_Switch1Button"; break;
    case ST_Swtich2: prefix = "System_Switch2Button"; break;
    case ST_COUNT: emit notifyFinished(false); return;
    }

    ++m_button;
    m_elapsedTimer.restart();

    m_moduleHolder->AddRunCommand("A|50");
    m_moduleHolder->AddFrameCapture(prefix + QString::number(m_button));
}

}
