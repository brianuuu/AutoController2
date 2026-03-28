#include "template.h"

namespace Program::CATEGORY
{

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

void NAME_NO_SPACE::OnCommandFinished(Module::Common::RunCommand* module)
{
	if (OnModuleErrorQuit(module)) return;
    m_moduleHolder->ClearModule(module);
	
	switch (m_state)
    {
    default:
    {
        UnhandedStateRunCommand();
        return;
    }
    }
}

void NAME_NO_SPACE::OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched)
{
	if (OnModuleErrorQuit(module)) return;
	
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
