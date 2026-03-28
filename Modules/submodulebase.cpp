#include "submodulebase.h"

namespace Module
{
SubModuleBase::SubModuleBase(QObject *parent)
    : ModuleBase{parent}
{
    m_moduleHolder = new Module::ModuleHolder(this);
    connect(m_moduleHolder, &Module::ModuleHolder::notifyCommandFinished, this, &SubModuleBase::OnCommandFinished);
    connect(m_moduleHolder, &Module::ModuleHolder::notifyResultMatched, this, &SubModuleBase::OnFrameCaptureMatched);
    connect(m_moduleHolder, &Module::ModuleHolder::notifyFinishQuit, this, &SubModuleBase::OnModuleFinishQuit);
    connect(m_moduleHolder, &Module::ModuleHolder::notifyErrorQuit, this, &SubModuleBase::OnModuleErrorQuit);
}

void SubModuleBase::OnModuleFinishQuit(ModuleBase *module)
{
    // finish sub module if this module is finished
    if (!module) return;

    m_result = module->GetResult();
    stop();
}

bool SubModuleBase::OnModuleErrorQuit(ModuleBase *module)
{
    // finish program if this module is errored out
    if (!module) return true;

    // module was already deleted
    if (!m_moduleHolder->m_modules.contains(module)) return true;

    int const result = module->GetResult();
    if (result < 0)
    {
        m_result = result;
        stop();
        return true;
    }

    return false;
}

}
