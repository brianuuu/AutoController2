#include "moduleholder.h"

namespace Module
{
void ModuleHolder::AddModule(Module::ModuleBase *module, bool finish)
{
    if (!module) return;

    // only allow 1 run command module
    if (module->IsCommand())
    {
        ClearRunCommand();
    }

    if (finish)
    {
        connect(module, &QThread::finished, this, &ModuleHolder::notifyFinishQuit);
    }
    else
    {
        connect(module, &QThread::finished, this, &ModuleHolder::notifyErrorQuit);
    }

    m_modules.insert(module);
    module->moveToThread(module);
    module->start(module->GetPriority());
}

void ModuleHolder::ClearModule(QObject *sender)
{
    Module::ModuleBase* module = qobject_cast<Module::ModuleBase*>(sender);
    ClearModule(module);
}

void ModuleHolder::ClearModule(Module::ModuleBase *module)
{
    if (!module || !m_modules.contains(module)) return;
    m_modules.remove(module);

    module->stop();
    module->wait();
    delete module;
}

void ModuleHolder::ClearModules()
{
    auto temp = m_modules;
    m_modules.clear();

    for (Module::ModuleBase* module : std::as_const(temp))
    {
        module->stop();
        module->wait();
        delete module;
    }
}

void ModuleHolder::ClearRunCommand()
{
    for (Module::ModuleBase* module : std::as_const(m_modules))
    {
        if (module->IsCommand())
        {
            ClearModule(module);
            break;
        }
    }
}
}
