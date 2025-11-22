#ifndef SYSTEM_H
#define SYSTEM_H

#include <qobject.h>

enum ButtonType
{
    BTN_None    = 0,
    BTN_A       = 1UL << 0,
    BTN_B       = 1UL << 1,
    BTN_X       = 1UL << 2,
    BTN_Y       = 1UL << 3,
    BTN_L       = 1UL << 4,
    BTN_R       = 1UL << 5,
    BTN_ZL      = 1UL << 6,
    BTN_ZR      = 1UL << 7,
    BTN_Plus    = 1UL << 8,
    BTN_Minus   = 1UL << 9,
    BTN_Home    = 1UL << 10,
    BTN_Capture = 1UL << 11,
    BTN_LClick  = 1UL << 12,
    BTN_LUp     = 1UL << 13,
    BTN_LDown   = 1UL << 14,
    BTN_LLeft   = 1UL << 15,
    BTN_LRight  = 1UL << 16,
    BTN_RClick  = 1UL << 17,
    BTN_RUp     = 1UL << 18,
    BTN_RDown   = 1UL << 19,
    BTN_RLeft   = 1UL << 20,
    BTN_RRight  = 1UL << 21,
    BTN_DUp     = 1UL << 22,
    BTN_DDown   = 1UL << 23,
    BTN_DLeft   = 1UL << 24,
    BTN_DRight  = 1UL << 25,
    BTN_Spam    = 1UL << 26, // does not apply to analog sticks

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
    if (str == "none" || str == "nothing")
    {
        return BTN_None;
    }

    QString const lower = str.toLower();
    for (int i = 0; i < BTN_COUNT; i++)
    {
        ButtonType type = (ButtonType)(1UL << i);
        if (lower == ButtonToString(type))
        {
            return type;
        }
    }

    return BTN_COUNT;
}

#endif // SYSTEM_H
