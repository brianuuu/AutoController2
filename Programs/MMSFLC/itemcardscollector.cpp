#include "itemcardscollector.h"

#include "Helpers/commandcollection.h"

namespace Program::MMSFLC
{

void ItemCardsCollector::PopulateSettings(QBoxLayout *layout)
{
    m_macroOnly = new Setting::SettingCheckBox("MacroOnly", "", false);
    AddSetting(layout, "Use Macro Only:", "Allows running program without camera, but goes through ciphers that has already been collected", m_macroOnly);
    connect(m_macroOnly, &QCheckBox::clicked, this, [this]{ OnCanRunChanged(); } );

    AddSpacer(layout);

    m_savedSettings.insert(m_macroOnly);
}

void ItemCardsCollector::Start()
{
    ProgramBase::Start();

    m_index = 0;
    StateStart();
}

void ItemCardsCollector::Stop()
{
    ProgramBase::Stop();
}

void ItemCardsCollector::OnCommandFinished(Module::Common::RunCommand* module)
{
	if (OnModuleErrorQuit(module)) return;
    m_moduleHolder->ClearModule(module);
	
	switch (m_state)
    {
    case State::ToItemCards:
    {
        QString command = "A|50,None|1000";
        if (m_index > 0)
        {
            command = "(LDown|50,None|50)" + QString::number(m_index) + "," + command;
        }

        m_state = SetState(State::Select, "Selecting item card no." + QString::number(m_index + 1));
        m_moduleHolder->AddRunCommand(command);
        m_moduleHolder->AddFrameCapture("System_CenterBlack");
        break;
    }
    case State::Select:
    {
        PrintLog("No black screen detected, cipher may have been collected already", LOG_Warning);

        ++m_index;
        if (m_index == 47)
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
        if (m_index == 47)
        {
            emit notifyFinished(true);
        }
        else
        {
            StateStart();
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

void ItemCardsCollector::OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched)
{
	if (OnModuleErrorQuit(module)) return;
	
	switch (m_state)
    {
    case State::Select:
    {
        if (matched)
        {
            m_state = SetState(State::Collect, "Black screen detected, collecting reward");
            m_moduleHolder->ClearModules();
            m_moduleHolder->AddRunCommand("B|2500");
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

void ItemCardsCollector::StateStart()
{
    if (m_macroOnly->isChecked())
    {
        QString command = CommandCollection::GetCommand("MMSF2_ToItemCards");
        if (m_index > 0)
        {
            command += ",(LDown|50,None|50)" + QString::number(m_index);
        }
        command += ",A|50,B|2500";

        m_state = SetState(State::Collect, "Collecting item card no." + QString::number(m_index + 1));
        m_moduleHolder->AddRunCommand(command);
    }
    else
    {
        m_state = SetState(State::ToItemCards, "Go to Item Cards");
        m_moduleHolder->AddRunCommand("MMSF2_ToItemCards");
    }
}

}
