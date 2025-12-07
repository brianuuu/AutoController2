#ifndef PROGRAMMANAGER_H
#define PROGRAMMANAGER_H

#include <QComboBox>
#include <QMessageBox>
#include <QListWidget>
#include <QWidget>

#include "../ui_mainwindow.h"
#include "Programs/programbase.h"

class ProgramManager : public QWidget
{
    Q_OBJECT

public:
    ProgramManager(QWidget* parent = nullptr) : QWidget(parent) {}
    static QString GetTypeID() { return "Program"; }
    void Initialize(Ui::MainWindow* ui);

    bool OnCloseEvent();

private slots:
    void OnCategoryChanged(QString const& category);
    void OnProgramChanged(QString const& name);
    void OnResetDefault();

private:
    void LoadSettings();
    void SaveSettings() const;

    template<class T>
    void RegisterProgram();
    void RemoveProgram();

private:
    // UI
    QComboBox*      m_programCategory = Q_NULLPTR;
    QListWidget*    m_programList = Q_NULLPTR;
    QBoxLayout*     m_programSettings = Q_NULLPTR;
    QPushButton*    m_btnResetDefault = Q_NULLPTR;
    QMap<QString, QStringList>  m_categoryToPrograms;

    using ProgramCtor = ProgramBase*(*)();
    QMap<QString, ProgramCtor> m_programCtors;

    // Members
    ProgramBase*    m_program = Q_NULLPTR;
};

#endif // PROGRAMMANAGER_H
