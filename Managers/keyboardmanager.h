#ifndef KEYBOARDMANAGER_H
#define KEYBOARDMANAGER_H

#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "../ui_mainwindow.h"
#include "Enums/system.h"

class KeyboardManager : public QWidget
{
    Q_OBJECT

public:
    KeyboardManager(QWidget* parent = nullptr) {}
    static QString GetTypeID() { return "Keyboard"; }
    void Initialize(Ui::MainWindow* ui);

    bool OnCloseEvent();
    bool OnInitShow();

protected:
    void closeEvent(QCloseEvent *event) override;

public slots:
    void OnShow();
    void OnResetDefault();
    void OnButtonClicked();

private:
    void SetButtonText(ButtonType type);

    void LoadSettings();
    void SaveSettings() const;

private:
    // Members
    bool            m_defaultShow = false;
    QPushButton*    m_btnRemap = Q_NULLPTR;
    QPushButton*    m_btnButton[BTN_COUNT - 1]; // skip spam
    QLabel*         m_labelButton[BTN_COUNT - 1];
    QLabel*         m_labelReset = Q_NULLPTR;

    QMap<ButtonType, int> m_typeToKeyMap;
    QMap<int, ButtonType> m_keyToTypeMap;
};

#endif // KEYBOARDMANAGER_H
