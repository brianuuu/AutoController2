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
    explicit RunCommand(QString const& nameOrCommand, bool isName, QObject *parent = nullptr);

    // from ModuleBase
    QString GetName() const override { return "Common-RunCommand"; }

    // from QThread
    void run() override;

    static QString GetDirectory() { return RESOURCES_PATH + "CommandCollection/"; }
    static QString GetExtension() { return ".command"; }
    static QRegularExpression GetRegularExpression() { return QRegularExpression("[A-Za-z0-9()|,\-\.]*"); }

signals:
    void notifyButton(quint32 buttonFlag, QPointF lStick = QPointF(), QPointF rStick = QPointF());

private:
    void SendCurrentCommand(bool isLoopCount = false);

private:
    SystemType      m_systemType = ST_COUNT;

    QString         m_name;
    QString         m_command;
    int             m_commandIndex = 0;
    QVector<int>    m_commandLoopCounts;
};
} // namespace Module

#endif // RUNCOMMAND_H
