#ifndef RUNCOMMAND_H
#define RUNCOMMAND_H

#include <QElapsedTimer>
#include <QPointF>

#include "../modulebase.h"
#include "Managers/managercollection.h"

namespace Module::Common
{
class RunCommand : public ModuleBase
{
    Q_OBJECT

public:
    explicit RunCommand(QString const& nameOrCommand, bool isName, QObject *parent = nullptr);

    // from ModuleBase
    QString GetName() const override { return "Common-RunCommand"; }

    // from QThread
    void run() override;

signals:
    void notifyButton(quint32 buttonFlag, QPointF lStick = QPointF(), QPointF rStick = QPointF());

private:
    void SendCurrentCommand(bool isLoopCount = false);

private:
    SerialManager*  m_serialManager = Q_NULLPTR;

    QString         m_name;
    QString         m_command;
    int             m_commandIndex = 0;
    QVector<int>    m_commandLoopCounts;
};
} // namespace Module

#endif // RUNCOMMAND_H
