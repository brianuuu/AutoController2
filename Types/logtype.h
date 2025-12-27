#ifndef LOGTYPE_H
#define LOGTYPE_H

#include <QColor>
#include <QString>

enum LogType
{
    LOG_Normal,
    LOG_Success,
    LOG_Warning,
    LOG_Error,
};

static QColor LogTypeToColor(LogType type)
{
    switch (type)
    {
    case LOG_Normal:    return QColor(0,0,0);
    case LOG_Success:   return QColor(0,170,0);
    case LOG_Warning:   return QColor(255,120,0);
    case LOG_Error:     return QColor(255,0,0);
    }

    return QColor(0,0,0);
}

static QString LogTypeDisplayText(LogType type)
{
    switch (type)
    {
    case LOG_Normal:    break;
    case LOG_Success:   return "[SUCCESS]";
    case LOG_Warning:   return "[WARNING]";
    case LOG_Error:     return "[ERROR]";
    }

    return "";
}

#endif // LOGTYPE_H
