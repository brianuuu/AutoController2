#include "runcommand.h"

#include "Helpers/jsonhelper.h"
#include "Managers/managercollection.h"
#include "Managers/profilemanager.h"
#include "Managers/serialmanager.h"

namespace Module::Common
{

RunCommand::RunCommand(const QString &nameOrCommand, uint startDelay)
    : ModuleBase(nullptr)
    , m_startDelay(startDelay)
{
    if (nameOrCommand.contains('|'))
    {
        m_command = nameOrCommand;
    }
    else
    {
        m_name = nameOrCommand;
    }

    ProfileManager* profileManager = ManagerCollection::GetManager<ProfileManager>();
    m_systemType = profileManager->GetSystemType();
}

void RunCommand::run()
{
    if (m_result < 0 || m_terminate) return;

    if (m_name.isEmpty())
    {
        if (m_startDelay > 0)
        {
            m_command = "None|" + QString::number(m_startDelay) + "," + m_command;
        }

        PrintLog("Running command = " + m_command);
    }
    else
    {
        QString const fullName = Module::Common::RunCommand::GetDirectory() + m_name + Module::Common::RunCommand::GetExtension();
        QJsonObject const object = JsonHelper::ReadJson(fullName);

        QVariant command;
        if (JsonHelper::ReadValue(object, SystemToString(m_systemType), command) && !command.toString().isEmpty())
        {
            m_command = command.toString();
        }
        else if (JsonHelper::ReadValue(object, "Default", command))
        {
            m_systemType = ST_COUNT;
            m_command = command.toString();
        }
        else
        {
            m_error = "Command \"" + m_name + "\" not found";
            m_result = -1;
            return;
        }

        if (m_startDelay > 0)
        {
            m_command = "None|" + QString::number(m_startDelay) + "," + m_command;
        }

        QString const type = m_systemType == ST_COUNT ? "Default" : SystemToString(m_systemType);
        PrintLog("Running command \"" + m_name + "\" for \"" + type + "\" = " + m_command);
    }

    if (!SerialManager::VerifyCommand(m_command, m_error))
    {
        m_result = -1;
    }

    SerialManager* serialManager = ManagerCollection::GetManager<SerialManager>();
    if (!serialManager->IsConnected())
    {
        m_result = -1;
        m_error = "Serial not connected";
    }

    connect(this, &RunCommand::notifyCommand, serialManager->GetHolder(), &SerialHolder::OnSendCommand);
    connect(this, &RunCommand::notifyClear, serialManager->GetHolder(), &SerialHolder::OnClearCommand);
    connect(serialManager->GetHolder(), &SerialHolder::notifyCommandFinished, this, [this]{ quit(); });
    emit notifyCommand(m_command);

    // wait for command finish signal
    exec();

    if (m_terminate)
    {
        // terminate, we have to stop current command
        emit notifyClear();
    }
    else
    {
        emit notifyFinished(this);
    }
}

}
