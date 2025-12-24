#include "joystickmanager.h"

#include "../ui_mainwindow.h"
#include "Managers/logmanager.h"
#include "Managers/keyboardmanager.h"

#define JOYSTICK_DETECT_INTERVAL 500
#define JOYSTICK_WATCH_INTERVAL 10
#define JOYSTICK_TRIGGER_THRESHOLD 0.5
#define JOYSTICK_DEADZONE 0.125

void JoystickManager::Initialize(Ui::MainWindow *ui)
{
    m_logManager = ManagerCollection::GetManager<LogManager>();

    // detect joystick
    connect(&m_watchTimer, &QTimer::timeout, this, &JoystickManager::OnWatchTimeout);

    KeyboardManager* keyboardManager = ManagerCollection::GetManager<KeyboardManager>();
    connect(this, &JoystickManager::notifyHasJoystick, keyboardManager, &KeyboardManager::OnUpdateStatus);
}

void JoystickManager::SetEnabled(bool enabled)
{
    if (m_enabled == enabled) return;
    m_enabled = enabled;

    if (m_enabled)
    {
        m_logManager->PrintLog("Global", "Gamepad enabled");
        m_watchTimer.setInterval(JOYSTICK_DETECT_INTERVAL);
        OnWatchTimeout();
        m_watchTimer.start();
    }
    else
    {
        m_id = UINT_MAX;
        ClearButtonFlags();
        emit notifyHasJoystick();

        m_logManager->PrintLog("Global", "Gamepad disabled");
        m_watchTimer.stop();
    }
}

void JoystickManager::OnWatchTimeout()
{
    JOYINFOEX info;
    info.dwSize = sizeof(info);
    info.dwFlags = JOY_RETURNALL;

    if (m_id == UINT_MAX)
    {
        for (quint32 i = 0; i < 16; i++)
        {
            if (joyGetPosEx(i, &info) == JOYERR_NOERROR)
            {
                m_id = i;

                m_logManager->PrintLog("Global", "Gamepad detected: ID = " + QString::number(i));
                m_watchTimer.setInterval(JOYSTICK_WATCH_INTERVAL);

                emit notifyHasJoystick();
                emit notifyChanged(m_buttonFlags, m_lStick, m_rStick);
                break;
            }
        }
    }
    else if (joyGetPosEx(m_id, &info) == JOYERR_NOERROR)
    {
        quint32 buttonFlags = 0;
        QPointF lStick = QPointF();
        QPointF rStick = QPointF();

        // D-pad
        switch (info.dwPOV)
        {
        case JOY_POVFORWARD:
            buttonFlags |= ButtonToFlag(BTN_DUp);
            break;
        case JOY_POVBACKWARD:
            buttonFlags |= ButtonToFlag(BTN_DDown);
            break;
        case JOY_POVLEFT:
            buttonFlags |= ButtonToFlag(BTN_DLeft);
            break;
        case JOY_POVRIGHT:
            buttonFlags |= ButtonToFlag(BTN_DRight);
            break;
        case (JOY_POVFORWARD + JOY_POVRIGHT) / 2:
            buttonFlags |= ButtonToFlag(BTN_DUp);
            buttonFlags |= ButtonToFlag(BTN_DRight);
            break;
        case (36000 + JOY_POVLEFT) / 2:
            buttonFlags |= ButtonToFlag(BTN_DUp);
            buttonFlags |= ButtonToFlag(BTN_DLeft);
            break;
        case (JOY_POVBACKWARD + JOY_POVRIGHT) / 2:
            buttonFlags |= ButtonToFlag(BTN_DDown);
            buttonFlags |= ButtonToFlag(BTN_DRight);
            break;
        case (JOY_POVBACKWARD + JOY_POVLEFT) / 2:
            buttonFlags |= ButtonToFlag(BTN_DDown);
            buttonFlags |= ButtonToFlag(BTN_DLeft);
            break;
        }

        // buttons
        if (info.dwButtons & JOY_BUTTON1) buttonFlags |= ButtonToFlag(BTN_B);
        if (info.dwButtons & JOY_BUTTON2) buttonFlags |= ButtonToFlag(BTN_A);
        if (info.dwButtons & JOY_BUTTON3) buttonFlags |= ButtonToFlag(BTN_Y);
        if (info.dwButtons & JOY_BUTTON4) buttonFlags |= ButtonToFlag(BTN_X);
        if (info.dwButtons & JOY_BUTTON5) buttonFlags |= ButtonToFlag(BTN_L);
        if (info.dwButtons & JOY_BUTTON6) buttonFlags |= ButtonToFlag(BTN_R);
        if (info.dwButtons & JOY_BUTTON7) buttonFlags |= ButtonToFlag(BTN_Minus);
        if (info.dwButtons & JOY_BUTTON8) buttonFlags |= ButtonToFlag(BTN_Plus);
        if (info.dwButtons & JOY_BUTTON9) buttonFlags |= ButtonToFlag(BTN_LClick);
        if (info.dwButtons & JOY_BUTTON10) buttonFlags |= ButtonToFlag(BTN_RClick);
        if (info.dwButtons & JOY_BUTTON11) buttonFlags |= ButtonToFlag(BTN_Home);

        // triggers
        qreal const trigger = NormalizeStickPos(info.dwZpos);
        if (trigger >= JOYSTICK_TRIGGER_THRESHOLD)
        {
            buttonFlags |= ButtonToFlag(BTN_ZL);
        }
        else if (trigger <= -JOYSTICK_TRIGGER_THRESHOLD)
        {
            buttonFlags |= ButtonToFlag(BTN_ZR);
        }

        // sticks
        qreal lx = NormalizeStickPos(info.dwXpos);
        if (qAbs(lx) < JOYSTICK_DEADZONE) lx = 0.0;
        qreal ly = -NormalizeStickPos(info.dwYpos);
        if (qAbs(ly) < JOYSTICK_DEADZONE) ly = 0.0;
        qreal rx = NormalizeStickPos(info.dwUpos);
        if (qAbs(rx) < JOYSTICK_DEADZONE) rx = 0.0;
        qreal ry = -NormalizeStickPos(info.dwRpos);
        if (qAbs(ry) < JOYSTICK_DEADZONE) ry = 0.0;
        lStick = QPointF(lx, ly);
        rStick = QPointF(rx, ry);

        // send message
        qreal constexpr threshold = 0.005;
        if (m_buttonFlags != buttonFlags
        || qAbs(m_lStick.x() - lStick.x()) >= threshold
        || qAbs(m_lStick.y() - lStick.y()) >= threshold
        || qAbs(m_rStick.x() - rStick.x()) >= threshold
        || qAbs(m_rStick.y() - rStick.y()) >= threshold
        )
        {
            emit notifyChanged(buttonFlags, lStick, rStick);
        }

        m_buttonFlags = buttonFlags;
        m_lStick = lStick;
        m_rStick = rStick;
    }
    else
    {
        // disconnected
        m_id = UINT_MAX;
        ClearButtonFlags();
        emit notifyHasJoystick();

        m_logManager->PrintLog("Global", "Gamepad disconnected", LOG_Warning);
        m_watchTimer.setInterval(JOYSTICK_DETECT_INTERVAL);
    }
}

qreal JoystickManager::NormalizeStickPos(DWORD value)
{
    return (qreal)value * 2.0 / 65535.0 - 1.0;
}

void JoystickManager::ClearButtonFlags()
{
    if (m_buttonFlags != 0 || m_lStick != QPointF() || m_rStick != QPointF())
    {
        m_buttonFlags = 0;
        m_lStick = QPointF();
        m_rStick = QPointF();

        emit notifyChanged(m_buttonFlags, m_lStick, m_rStick);
    }
}
