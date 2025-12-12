#ifndef JOYSTICKMANAGER_H
#define JOYSTICKMANAGER_H

#include <QTimer>
#include <QWidget>

#include <windows.h>
#include <joystickapi.h>

#include "../ui_mainwindow.h"
#include "Managers/managercollection.h"

class JoystickManager : public QWidget
{
    Q_OBJECT
public:
    explicit JoystickManager(QWidget *parent = nullptr) : QWidget(parent) {}
    static QString GetTypeID() { return "Joystick"; }
    void Initialize(Ui::MainWindow* ui);

    void SetEnabled(bool enabled);

signals:
    void notifyChanged(quint32 buttonFlag, QPointF lStick, QPointF rStick);

private slots:
    void OnWatchTimeout();

private:
    static qreal NormalizeStickPos(DWORD value);
    void ClearButtonFlags();

private:
    LogManager* m_logManager = Q_NULLPTR;

    bool    m_enabled = false;
    quint32 m_id = UINT_MAX;
    QTimer  m_watchTimer;

    quint32 m_buttonFlags = 0;
    QPointF m_lStick = QPointF();
    QPointF m_rStick = QPointF();
};

#endif // JOYSTICKMANAGER_H
