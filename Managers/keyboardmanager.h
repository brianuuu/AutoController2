#ifndef KEYBOARDMANAGER_H
#define KEYBOARDMANAGER_H

#include <QCheckBox>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QWidget>

#include "../ui_mainwindow.h"
#include "Enums/system.h"
#include "Helpers/stickpainter.h"
#include "Managers/managercollection.h"

class KeyboardManager : public QWidget
{
    Q_OBJECT

public:
    explicit KeyboardManager(QWidget* parent = nullptr) {}
    static QString GetTypeID() { return "Keyboard"; }
    void Initialize(Ui::MainWindow* ui);

    bool OnCloseEvent();
    bool OnInitShow();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void notifyUserInput(quint32 buttonFlag, QPointF lStick = QPointF(), QPointF rStick = QPointF());

public slots:
    void OnDisplayButton(quint32 buttonFlag, QPointF lStick = QPointF(), QPointF rStick = QPointF());
    void OnUpdateStatus();
    void OnShow();

private slots:
    void OnResetDefault();
    void OnButtonClicked();
    void OnJoystickEnabled(Qt::CheckState state);
    void OnJoystickChanged(quint32 buttonFlag, QPointF lStick, QPointF rStick);

private:
    void SetButtonText(ButtonType type);
    void UpdateButtonMap(QPushButton* button, int key);
    void UpdateButtonFlags(int key, bool pressed);
    void ClearButtonFlags();

    static void ButtonRemap(QPushButton* button);
    static void ButtonPressed(QPushButton* button);
    static void ButtonReleased(QPushButton* button);
    static void ButtonSpam(QPushButton* button);

    void LoadSettings();
    void SaveSettings() const;

private:
    // Managers
    JoystickManager*m_joystickManager = Q_NULLPTR;
    ProgramManager* m_programManager = Q_NULLPTR;
    SerialManager*  m_serialManager = Q_NULLPTR;
    VlcManager*     m_vlcManager = Q_NULLPTR;

    // UI
    QCheckBox*      m_cbJoystick = Q_NULLPTR;
    QPushButton*    m_btnRemap = Q_NULLPTR;
    QPushButton*    m_btnButton[BTN_COUNT - 1]; // skip spam
    QLabel*         m_labelButton[BTN_COUNT - 1];
    QLabel*         m_labelReset = Q_NULLPTR;
    QLabel*         m_labelStatus = Q_NULLPTR;
    QLabel*         m_labelJoystick = Q_NULLPTR;
    StickPainter*   m_leftStick = Q_NULLPTR;
    StickPainter*   m_rightStick = Q_NULLPTR;

    // Members
    bool                    m_defaultShow = false;
    bool                    m_inputActive = false;
    bool                    m_joystickActive = false;
    quint32                 m_buttonFlag = 0;
    QMap<ButtonType, int>   m_typeToKeyMap;
    QMap<int, ButtonType>   m_keyToTypeMap;
};

#endif // KEYBOARDMANAGER_H
