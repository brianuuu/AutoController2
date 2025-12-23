#ifndef PROGRAMMANAGER_H
#define PROGRAMMANAGER_H

#include <QComboBox>
#include <QMessageBox>
#include <QListWidget>
#include <QWidget>

#include "../ui_mainwindow.h"
#include "Managers/managercollection.h"
#include "Programs/programbase.h"

class ProgramManager : public QWidget
{
    Q_OBJECT

public:
    ProgramManager(QWidget* parent = nullptr) : QWidget(parent) {}
    static QString GetTypeID() { return "Program"; }
    void Initialize(Ui::MainWindow* ui);

    bool OnCloseEvent();

    bool AllowKeyboardInput() const { return !IsRunning() || !m_program->RequireSerial() || m_program->CanControlWhileRunning(); }
    bool IsRunning() const { return m_program && m_program->IsRunning(); }

signals:
    void notifyStartStop();

private slots:
    void OnCategoryChanged(QString const& category);
    void OnProgramChanged(QString const& name);
    void OnCanRunChanged(bool canRun);
    void OnProgramStartStop();
    void OnProgramFinished(int result);
    void OnResetDefault();

private:
    void LoadSettings();
    void SaveSettings() const;

    void StartProgram();
    void StopProgram();

    template<class T>
    void RegisterProgram();
    void RemoveProgram();

private:
    // Managers
    LogManager*     m_logManager = Q_NULLPTR;

    // UI
    QComboBox*      m_programCategory = Q_NULLPTR;
    QListWidget*    m_programList = Q_NULLPTR;
    QWidget*        m_settingsParent = Q_NULLPTR;
    QBoxLayout*     m_settingsLayout = Q_NULLPTR;
    QPushButton*    m_btnStart = Q_NULLPTR;
    QPushButton*    m_btnResetDefault = Q_NULLPTR;
    QLabel*         m_labelDescription = Q_NULLPTR;
    QLabel*         m_labelSerial = Q_NULLPTR;
    QLabel*         m_labelCamera = Q_NULLPTR;
    QLabel*         m_labelAudio = Q_NULLPTR;
    QMap<QString, QStringList>  m_categoryToPrograms;

    using ProgramCtor = Program::ProgramBase*(*)();
    QMap<QString, ProgramCtor> m_programCtors;

    // Members
    Program::ProgramBase*   m_program = Q_NULLPTR;
};

#endif // PROGRAMMANAGER_H
