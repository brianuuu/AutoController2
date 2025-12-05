#ifndef SYSTEM_H
#define SYSTEM_H

#include <qcolor.h>
#include <qobject.h>

enum ButtonType
{
    BTN_None,
    BTN_A,
    BTN_B,
    BTN_X,
    BTN_Y,
    BTN_L,
    BTN_R,
    BTN_ZL,
    BTN_ZR,
    BTN_Plus,
    BTN_Minus,
    BTN_Home,
    BTN_Capture,
    BTN_LClick,
    BTN_LUp,
    BTN_LDown,
    BTN_LLeft,
    BTN_LRight,
    BTN_RClick,
    BTN_RUp,
    BTN_RDown,
    BTN_RLeft,
    BTN_RRight,
    BTN_DUp,
    BTN_DDown,
    BTN_DLeft,
    BTN_DRight,
    BTN_Spam, // does not apply to analog sticks

    BTN_COUNT = 28
};

static QString ButtonToString(ButtonType button)
{
    switch (button)
    {
    case BTN_None:      return "none";
    case BTN_A:         return "a";
    case BTN_B:         return "b";
    case BTN_X:         return "x";
    case BTN_Y:         return "y";
    case BTN_L:         return "l";
    case BTN_R:         return "r";
    case BTN_ZL:        return "zl";
    case BTN_ZR:        return "zr";
    case BTN_Plus:      return "plus";
    case BTN_Minus:     return "minus";
    case BTN_Home:      return "home";
    case BTN_Capture:   return "capture";
    case BTN_LClick:    return "lclick";
    case BTN_LUp:       return "lup";
    case BTN_LDown:     return "ldown";
    case BTN_LLeft:     return "lleft";
    case BTN_LRight:    return "lright";
    case BTN_RClick:    return "rclick";
    case BTN_RUp:       return "rup";
    case BTN_RDown:     return "rdown";
    case BTN_RLeft:     return "rleft";
    case BTN_RRight:    return "rright";
    case BTN_DUp:       return "dup";
    case BTN_DDown:     return "ddown";
    case BTN_DLeft:     return "dleft";
    case BTN_DRight:    return "dright";
    case BTN_Spam:      return "spam";
    default:            return "invalid";
    }
}

static ButtonType StringToButton(QString const& str)
{
    QString const lower = str.toLower();
    if (lower == "none" || lower == "nothing")
    {
        return BTN_None;
    }

    for (int i = 0; i < BTN_COUNT; i++)
    {
        ButtonType type = (ButtonType)i;
        if (lower == ButtonToString(type))
        {
            return type;
        }
    }

    return BTN_COUNT;
}

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

#endif // SYSTEM_H
