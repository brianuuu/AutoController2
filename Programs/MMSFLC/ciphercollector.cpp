#include "ciphercollector.h"

namespace Program::MMSFLC
{

void CipherCollector::PopulateSettings(QBoxLayout *layout)
{
    m_game = new Setting::SettingComboBox("Game", {"Mega Man Star Force 1", "Mega Man Star Force 2"});
    AddSetting(layout, "Game:", "Choose game", m_game);

    m_macroOnly = new Setting::SettingCheckBox("MacroOnly", "", false);
    AddSetting(layout, "Use Macro Only:", "Allows running program without camera, but goes through ciphers that has already been collected", m_macroOnly);
    connect(m_macroOnly, &QCheckBox::clicked, this, [this]{ OnCanRunChanged(); } );

    AddSpacer(layout);

    m_savedSettings.insert(m_game);
    m_savedSettings.insert(m_macroOnly);
}

void CipherCollector::Start()
{
    ProgramBase::Start();

    m_index = 0;
    StateToCipherList();
}

void CipherCollector::Stop()
{
    ProgramBase::Stop();
}

void CipherCollector::OnCommandFinished(Module::Common::RunCommand* module)
{
	if (OnModuleErrorQuit(module)) return;
    m_moduleHolder->ClearModule(module);
	
	switch (m_state)
    {
    case State::ToCipherList:
    {
        m_moduleHolder->AddFrameCapture("MMSFLC_CipherSelect");
        m_elapsedTimer.restart();
        break;
    }
    case State::Select:
    {
        PrintLog("No black screen detected, cipher may have been collected already", LOG_Warning);

        ++m_index;
        if (m_index == 60)
        {
            emit notifyFinished(true);
        }
        else
        {
            m_state = SetState(State::Select, "Selecting cipher no." + QString::number(m_index + 1));
            m_moduleHolder->AddRunCommand("LDown|50,A|50,None|1000");
        }
        break;
    }
    case State::Collect:
    {
        ++m_index;
        if (m_index == 60)
        {
            emit notifyFinished(true);
        }
        else
        {
            StateToCipherList();
        }
        break;
    }
    default:
    {
        UnhandedStateRunCommand();
        return;
    }
    }
}

void CipherCollector::OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched)
{
	if (OnModuleErrorQuit(module)) return;
	
	switch (m_state)
    {
    case State::ToCipherList:
    {
        if (m_elapsedTimer.elapsed() > 2000)
        {
            emit notifyFinished(false, "Unable to detect cipher list for too long");
        }
        else if (matched)
        {
            m_moduleHolder->ClearModules();

            QString command = "A|50,None|1000";
            if (m_index > 0)
            {
                command = "(LDown|50,None|50)" + QString::number(m_index) + "," + command;
            }

            m_state = SetState(State::Select, "Selecting cipher no." + QString::number(m_index + 1));
            m_moduleHolder->AddRunCommand(command);
            m_moduleHolder->AddFrameCapture("System_CenterBlack");
        }
        break;
    }
    case State::Select:
    {
        if (matched)
        {
            m_state = SetState(State::Collect, "Black screen detected, collecting reward");
            m_moduleHolder->ClearModules();
            m_moduleHolder->AddRunCommand("B|Spam|5500");
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

void CipherCollector::StateToCipherList()
{
    m_state = SetState(State::ToCipherList, "Go to Cipher List");
    m_moduleHolder->AddRunCommand("MMSF1_ToCipherList");
}

}
