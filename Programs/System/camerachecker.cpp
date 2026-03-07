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
    AddFrameCapture("System_HomeTheme");
}

void CameraChecker::Stop()
{
    ProgramBase::Stop();
}

void CameraChecker::OnCommandFinished()
{
    if (OnModuleErrorQuit()) return;
    ClearModule(sender());

    switch (m_state)
    {
    case State::SystemSetting:
    {
        m_state = SetState(State::ButtonMenu, "Go to button test menu");
        AddRunCommand("System_ButtonTest", 0);
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

void CameraChecker::OnFrameCaptureMatched(bool matched)
{
    if (OnModuleErrorQuit()) return;
    Module::Common::FrameCapture* module = qobject_cast<Module::Common::FrameCapture*>(sender());

    switch (m_state)
    {
    case State::DetectTheme:
    {
        m_color = module->GetResultColor();
        ClearModules();

        bool matchSystemType = true;
        if (m_color == ThemeLight)
        {
            PrintLog("Light Theme detected (Make sure System type in Global Settings is set correctly)", LOG_Important);
        }
        else if (m_color == ThemeDark1)
        {
            PrintLog("Nintendo Switch 1 Dark Theme detected", LOG_Important);
            matchSystemType = m_profileManager->GetSystemType() == ST_Swtich1;
        }
        else if (m_color == ThemeDark2)
        {
            PrintLog("Nintendo Switch 2 Dark Theme detected", LOG_Important);
            matchSystemType = m_profileManager->GetSystemType() == ST_Swtich2;
        }
        else
        {
            PrintLog("Unable to determine Nintendo Switch theme from color " + m_color.name() + ", display color from capture card maybe inaccurate. (Is HDR option turned off for Nintendo Switch 2?)", LOG_Warning);
        }

        if (!matchSystemType)
        {
            emit notifyFinished(false, "Incorrect System type set in Global Settings");
        }
        else
        {
            m_state = SetState(State::SystemSetting, "Go to system settings");
            AddRunCommand("System_Settings", 0);
        }
        break;
    }
    case State::ButtonTest:
    {
        QColor const color = module->GetResultColor();
        if (color != m_color)
        {
            ClearModules();

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
                AddRunCommand("Home|50,None|100");
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

    AddRunCommand("A|50");
    AddFrameCapture(prefix + QString::number(m_button));
}

}
