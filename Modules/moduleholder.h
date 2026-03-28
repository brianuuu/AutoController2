#ifndef MODULEHOLDER_H
#define MODULEHOLDER_H

#include <QObject>

#include "Modules/Common/framecapture.h"
#include "Modules/Common/runcommand.h"
#include "Modules/modulebase.h"

class ModuleHolder : public QObject
{
    Q_OBJECT
public:
    explicit ModuleHolder(QObject *parent = nullptr) : QObject(parent) {}
    ~ModuleHolder() { ClearModules(); }

protected slots:
    virtual void OnModuleFinishQuit() {}
    virtual bool OnModuleErrorQuit() { return true; }

    virtual void OnCommandFinished() {}
    virtual void OnFrameCaptureMatched(bool matched) {}

protected:
    void AddModule(Module::ModuleBase* module, bool finish = false);
    void ClearModule(QObject* sender);
    void ClearModule(Module::ModuleBase* module);
    void ClearModules();
    void ClearRunCommand();

    template<typename... Args>
    Module::Common::RunCommand* AddRunCommand(Args... args)
    {
        Module::Common::RunCommand* module = new Module::Common::RunCommand(args...);
        connect(module, &Module::Common::RunCommand::notifyFinished, this, &ModuleHolder::OnCommandFinished);
        AddModule(module);
        return module;
    }

    template<typename... Args>
    Module::Common::FrameCapture* AddFrameCapture(Args... args)
    {
        Module::Common::FrameCapture* module = new Module::Common::FrameCapture(args...);
        connect(module, &Module::Common::FrameCapture::notifyResultMatched, this, &ModuleHolder::OnFrameCaptureMatched);
        AddModule(module);
        return module;
    }

protected:
    QSet<Module::ModuleBase*> m_modules;
};

#endif // MODULEHOLDER_H
