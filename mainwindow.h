#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "Managers/managercollection.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void LoadSettings();
    void SaveSettings() const;

private:
    Ui::MainWindow *ui;

    // managers
    LogManager*         m_logManager = Q_NULLPTR;
    JoystickManager*    m_joystickManager = Q_NULLPTR;
    KeyboardManager*    m_keyboardManager = Q_NULLPTR;
    SerialManager*      m_serialManager = Q_NULLPTR;
    VlcManager*         m_vlcManager = Q_NULLPTR;
    ProgramManager*     m_programManager = Q_NULLPTR;
};
#endif // MAINWINDOW_H
