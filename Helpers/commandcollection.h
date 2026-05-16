#ifndef COMMANDCOLLECTION_H
#define COMMANDCOLLECTION_H

#include <QMap>
#include <QString>
#include <QRegularExpression>

#include "Types/systemtype.h"
#include "defines.h"

class CommandCollection
{
private:
    static CommandCollection& instance();

public:
    static QString GetDirectory() { return RESOURCES_PATH + "CommandCollection/"; }
    static QString GetExtension() { return ".command"; }
    static QRegularExpression GetRegularExpression() { return QRegularExpression("[A-Za-z0-9()|,\-\.]*"); }

    static void CacheCommand(QString const& name);
    static QString const& GetCommand(QString const& name);
    static QString const& GetCommand(QString const& name, SystemType type);

private:
    using SystemCommand = std::map<SystemType, QString>;
    std::map<QString, SystemCommand> m_collection;
};

#endif // COMMANDCOLLECTION_H
