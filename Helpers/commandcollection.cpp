#include "commandcollection.h"

#include "Helpers/jsonhelper.h"
#include "Managers/managercollection.h"
#include "Managers/profileManager.h"

CommandCollection& CommandCollection::instance()
{
    static CommandCollection collection;
    return collection;
}

void CommandCollection::CacheCommand(const QString &name)
{
    QString const fullName = GetDirectory() + name + GetExtension();
    QJsonObject const object = JsonHelper::ReadJson(fullName);

    QVariant command;
    for (int i = 0; i < ST_COUNT; i++)
    {
        SystemType const type = (SystemType)i;
        if (JsonHelper::ReadValue(object, SystemToString(type), command))
        {
            instance().m_collection[name][type] = command.toString();
        }
    }

    if (JsonHelper::ReadValue(object, "Default", command))
    {
        instance().m_collection[name][ST_COUNT] = command.toString();
    }
}

const QString &CommandCollection::GetCommand(const QString &name)
{
    ProfileManager* profileManager = ManagerCollection::GetManager<ProfileManager>();
    SystemType const type = profileManager->GetSystemType();

    QString const& command = GetCommand(name, type);
    if (command.isEmpty())
    {
        return GetCommand(name, ST_COUNT);
    }
    else
    {
        return command;
    }
}

const QString &CommandCollection::GetCommand(const QString &name, SystemType type)
{
    if (!instance().m_collection.count(name))
    {
        CacheCommand(name);
    }

    return instance().m_collection[name][type];
}
