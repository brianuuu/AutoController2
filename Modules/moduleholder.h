#ifndef MODULEHOLDER_H
#define MODULEHOLDER_H

#include <QObject>

#include "Modules/Common/framecapture.h"
#include "Modules/Common/runcommand.h"
#include "Modules/modulebase.h"

namespace Module
{
class ModuleHolder : public QObject
{
    Q_OBJECT
public:
    explicit ModuleHolder(QObject *parent = nullptr) : QObject(parent) {}
    ~ModuleHolder() { ClearModules(); }

signals:
    void notifyCommandFinished(Module::Common::RunCommand* module);
    void notifyResultMatched(Module::Common::FrameCapture* module, bool matched);
    void notifyFinishQuit(Module::ModuleBase* module);
    void notifyErrorQuit(Module::ModuleBase* module);

public:
    void AddModule(Module::ModuleBase* module, bool finish = false);
    void ClearModule(QObject* sender);
    void ClearModule(Module::ModuleBase* module);
    void ClearModules();
    void ClearRunCommand();

    template<typename... Args>
    Module::Common::RunCommand* AddRunCommand(Args... args)
    {
        Module::Common::RunCommand* module = new Module::Common::RunCommand(args...);
        connect(module, &Module::Common::RunCommand::notifyFinished, this, &ModuleHolder::notifyCommandFinished);
        AddModule(module);
        return module;
    }

    template<typename... Args>
    Module::Common::FrameCapture* AddFrameCapture(Args... args)
    {
        Module::Common::FrameCapture* module = new Module::Common::FrameCapture(args...);
        connect(module, &Module::Common::FrameCapture::notifyResultMatched, this,&ModuleHolder::notifyResultMatched);
        AddModule(module);
        return module;
    }

public:
    QSet<Module::ModuleBase*> m_modules;
};
}

#endif // MODULEHOLDER_H
