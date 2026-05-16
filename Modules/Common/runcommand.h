#ifndef RUNCOMMAND_H
#define RUNCOMMAND_H

#include <QElapsedTimer>
#include <QPointF>

#include "../modulebase.h"
#include "Types/systemtype.h"
#include "defines.h"

namespace Module::Common
{
class RunCommand : public ModuleBase
{
    Q_OBJECT
public:
    explicit RunCommand(QString const& nameOrCommand, uint startDelay = 0);

    // from ModuleBase
    QString GetName() const override { return "Common-RunCommand"; }
    bool IsCommand() const override { return true; }

    // from QThread
    void run() override;

signals:
    void notifyCommand(QString const& command);
    void notifyFinished(Module::Common::RunCommand* module);
    void notifyClear();

private:
    SystemType      m_systemType = ST_COUNT;

    QString         m_name;
    QString         m_command;
    uint            m_startDelay = 0;
};
} // namespace Module

#endif // RUNCOMMAND_H
