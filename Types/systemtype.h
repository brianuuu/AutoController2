#ifndef SYSTEMTYPE_H
#define SYSTEMTYPE_H

#include <QString>
#include <QStringList>

enum SystemType
{
    ST_Swtich1,
    ST_Swtich2,

    ST_COUNT
};

static QString SystemToString(SystemType system)
{
    switch (system)
    {
    case ST_Swtich1:    return "Switch1";
    case ST_Swtich2:    return "Switch2";
    default:            return "Unknown";
    }
}

static QString SystemToFullString(SystemType system)
{
    switch (system)
    {
    case ST_Swtich1:    return "Nintendo Switch";
    case ST_Swtich2:    return "Nintendo Switch 2";
    default:            return "Unknown";
    }
}

static QStringList SystemStringList()
{
    QStringList list;
    for (int i = 0; i < ST_COUNT; i++)
    {
        list << SystemToFullString((SystemType)i);
    }
    return list;
}

#endif // SYSTEMTYPE_H
