#ifndef LOGTYPE_H
#define LOGTYPE_H

#include <QColor>
#include <QGuiApplication>
#include <QPalette>
#include <QString>

enum LogType
{
    LOG_Normal,
    LOG_Success,
    LOG_Warning,
    LOG_Error,
    LOG_Important,
    LOG_Shiny,
    LOG_State,
};

static QColor LogTypeToColor(LogType type)
{
    switch (type)
    {
    case LOG_Normal:    return QGuiApplication::palette().windowText().color();
    case LOG_Success:   return QColor(0,170,0);
    case LOG_Warning:   return QColor(255,120,0);
    case LOG_Error:     return QColor(255,0,0);
    case LOG_Important: return QColor(255,0,255);
    case LOG_Shiny:     return QColor(255,255,0);
    case LOG_State:     return QColor(0,170,255);
    }

    return QColor(0,0,0);
}

static QString LogTypeDisplayText(LogType type)
{
    switch (type)
    {
    case LOG_Success:   return "[SUCCESS]";
    case LOG_Warning:   return "[WARNING]";
    case LOG_Error:     return "[ERROR]";
    case LOG_Important: return "[IMPORTANT]";
    default: break;
    }

    return "";
}

#endif // LOGTYPE_H
