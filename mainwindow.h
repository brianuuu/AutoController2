#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "Managers/audiomanager.h"
#include "Managers/videomanager.h"
#include "Managers/serialmanager.h"

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

    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;

    // managers
    SerialManager*  m_serialManager = Q_NULLPTR;
    AudioManager*   m_audioManager = Q_NULLPTR;
    VideoManager*   m_videoManager = Q_NULLPTR;
};
#endif // MAINWINDOW_H
