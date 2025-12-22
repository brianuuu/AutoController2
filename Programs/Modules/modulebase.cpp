#include "modulebase.h"

#include "Managers/managercollection.h"
#include "Managers/logmanager.h"

namespace Module
{

ModuleBase::ModuleBase(QObject *parent) : QThread(parent)
{
    connect(this, &ModuleBase::started, this, &ModuleBase::OnStarted, Qt::DirectConnection);
    connect(this, &ModuleBase::finished, this, &ModuleBase::OnFinished, Qt::DirectConnection);

    LogManager* logManager = ManagerCollection::GetManager<LogManager>();
    connect(this, &ModuleBase::notifyLog, logManager, &LogManager::PrintLog);
}

void ModuleBase::OnStarted() const
{
    PrintLog("Module started");
}

void ModuleBase::OnFinished() const
{
    if (m_terminate)
    {
        PrintLog("Module terminated", LOG_Warning);
        return;
    }

    PrintLog("Module finished with result = " + QString::number(m_result), m_result < 0 ? LOG_Error : LOG_Normal);
    if (m_result < 0)
    {
        PrintLog(m_error, LOG_Error);
    }
}

void ModuleBase::PrintLog(const QString &log, LogType type) const
{
    emit notifyLog(GetName(), log, type);
}

}
