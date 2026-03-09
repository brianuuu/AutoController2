#include "template.h"

namespace Program::CATEGORY
{

NAME_NO_SPACE::NAME_NO_SPACE(QObject *parent)
    : ProgramBase{parent}
{
    
}

void NAME_NO_SPACE::PopulateSettings(QBoxLayout *layout)
{
    AddSpacer(layout);
}

void NAME_NO_SPACE::Start()
{
    ProgramBase::Start();
}

void NAME_NO_SPACE::Stop()
{
    ProgramBase::Stop();
}

void NAME_NO_SPACE::OnCommandFinished()
{
	if (OnModuleErrorQuit()) return;
    ClearModule(sender());
	
	switch (m_state)
    {
    default:
    {
        UnhandedStateRunCommand();
        return;
    }
    }
}

void NAME_NO_SPACE::OnFrameCaptureMatched(bool matched)
{
	if (OnModuleErrorQuit()) return;
	
	switch (m_state)
    {
    default:
    {
        UnhandedStateFrameCapture();
        return;
    }
    }
}

}
