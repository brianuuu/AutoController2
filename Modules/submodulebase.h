#ifndef SUBMODULEBASE_H
#define SUBMODULEBASE_H

#include "Modules/moduleholder.h"
#include "modulebase.h"

namespace Module
{
class SubModuleBase : public ModuleBase
{
    Q_OBJECT
public:
    explicit SubModuleBase(QObject *parent = nullptr);

signals:
    void notifyResult(Module::SubModuleBase* module, int result);

protected slots:
    void OnModuleFinishQuit(Module::ModuleBase* module);
    bool OnModuleErrorQuit(Module::ModuleBase* module);

    virtual void OnCommandFinished(Module::Common::RunCommand* module) {}
    virtual void OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched) {}

protected:
    ModuleHolder* m_moduleHolder = Q_NULLPTR;
};

}

#endif // SUBMODULEBASE_H
