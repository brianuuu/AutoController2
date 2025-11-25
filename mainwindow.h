#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "Managers/serialmanager.h"
#include "Managers/vlcmanager.h"

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
    VlcManager*     m_vlcManager = Q_NULLPTR;
};
#endif // MAINWINDOW_H
