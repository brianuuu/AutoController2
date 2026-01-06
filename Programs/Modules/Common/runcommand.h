#ifndef RUNCOMMAND_H
#define RUNCOMMAND_H

#include <QElapsedTimer>
#include <QPointF>
#include <QRegularExpression>

#include "../modulebase.h"
#include "Types/systemtype.h"
#include "defines.h"

namespace Module::Common
{
class RunCommand : public ModuleBase
{
    Q_OBJECT
public:
    explicit RunCommand(QString const& command);
    explicit RunCommand(QString const& name, uint startDelay);

    // from ModuleBase
    QString GetName() const override { return "Common-RunCommand"; }

    // from QThread
    void run() override;

    static QString GetDirectory() { return RESOURCES_PATH + "CommandCollection/"; }
    static QString GetExtension() { return ".command"; }
    static QRegularExpression GetRegularExpression() { return QRegularExpression("[A-Za-z0-9()|,\-\.]*"); }

signals:
    void notifyCommand(QString const& command);
    void notifyClear();

private:
    SystemType      m_systemType = ST_COUNT;

    QString         m_name;
    QString         m_command;
    uint            m_startDelay;
};
} // namespace Module

#endif // RUNCOMMAND_H
