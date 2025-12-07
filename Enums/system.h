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

static quint32 ButtonToFlag(ButtonType button)
{
    if (button == BTN_None || button == BTN_COUNT) return 0;
    return 1UL << (button - 1);
}

static QString ButtonToFullString(ButtonType button)
{
    switch (button)
    {
    case BTN_None:      return "None";
    case BTN_A:         return "A Button";
    case BTN_B:         return "B Button";
    case BTN_X:         return "X Button";
    case BTN_Y:         return "Y Button";
    case BTN_L:         return "L Button";
    case BTN_R:         return "R Button";
    case BTN_ZL:        return "ZL Button";
    case BTN_ZR:        return "ZR Button";
    case BTN_Plus:      return "Plus Button";
    case BTN_Minus:     return "Minus Button";
    case BTN_Home:      return "Home Button";
    case BTN_Capture:   return "Capture Button";
    case BTN_LClick:    return "L-Stick Click";
    case BTN_LUp:       return "L-Stick Up";
    case BTN_LDown:     return "L-Stick Down";
    case BTN_LLeft:     return "L-Stick Left";
    case BTN_LRight:    return "L-Stick Right";
    case BTN_RClick:    return "R-Stick Click";
    case BTN_RUp:       return "R-Stick Up";
    case BTN_RDown:     return "R-Stick Down";
    case BTN_RLeft:     return "R-Stick Left";
    case BTN_RRight:    return "R-Stick Right";
    case BTN_DUp:       return "D-Pad Up";
    case BTN_DDown:     return "D-Pad Down";
    case BTN_DLeft:     return "D-Pad Left";
    case BTN_DRight:    return "D-Pad Right";
    case BTN_Spam:      return "Spam";
    default:            return "Invalid";
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

static quint32 StringToButtonFlag(QString const& str)
{
    return ButtonToFlag(StringToButton(str));
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
