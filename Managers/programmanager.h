#ifndef PROGRAMMANAGER_H
#define PROGRAMMANAGER_H

#include <QComboBox>
#include <QDesktopServices>
#include <QMessageBox>
#include <QListWidget>
#include <QSettings>
#include <QShortcut>
#include <QWidget>

#include "Managers/managercollection.h"
#include "Programs/programbase.h"

namespace Ui { class MainWindow; }

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

    QString GetCurrentCategory() const { return m_programCategory->currentText(); }
    QString GetCurrentProgram() const { return m_programList->currentItem()->text(); }
    QString GetUpTimeString() const { return m_labelUpTime->text(); }
    QString GetStatsString() const { return m_labelStats->text(); }
    qint64 GetUpTime() const { return m_startTime.secsTo(QDateTime::currentDateTime()); }

    void RegisterStat(Stat& stat);

signals:
    void notifyStartStop();

private slots:
    void OnCategoryChanged(QString const& category);
    void OnProgramChanged(QString const& name);
    void OnCanRunChanged(bool canRun);
    void OnProgramStartStop();
    void OnProgramFinished(bool success, QString msg = "");
    void OnResetDefault();
    void OnManualOpen();
    void OnUpTimeUpdate();

    void OnStatsEdit();
    void OnStatsReset();

private:
    void LoadSettings();
    void SaveSettings() const;

    void StartProgram();
    void StopProgram();

    template<class T>
    void RegisterProgram();
    void RemoveProgram();
    bool HasProgramRun(QString const& name) const;

    void ReadStat(Stat& stat);
    void UpdateStats() const;
    void SaveStats();
    void ClearStats();
    void MigrateStatsToJson();

private:
    // Managers
    LogManager*     m_logManager = Q_NULLPTR;
    DiscordManager* m_discordManager = Q_NULLPTR;
    ProfileManager* m_profileManager = Q_NULLPTR;
    VideoManager*   m_videoManager = Q_NULLPTR;

    // UI
    QComboBox*      m_programCategory = Q_NULLPTR;
    QListWidget*    m_programList = Q_NULLPTR;
    QWidget*        m_settingsParent = Q_NULLPTR;
    QBoxLayout*     m_settingsLayout = Q_NULLPTR;
    QPushButton*    m_btnStart = Q_NULLPTR;
    QPushButton*    m_btnResetDefault = Q_NULLPTR;
    QPushButton*    m_btnManual = Q_NULLPTR;
    QLabel*         m_labelDescription = Q_NULLPTR;
    QLabel*         m_labelSerial = Q_NULLPTR;
    QLabel*         m_labelCamera = Q_NULLPTR;
    QLabel*         m_labelAudio = Q_NULLPTR;
    QLabel*         m_labelUpTime = Q_NULLPTR;
    QMap<QString, QStringList>  m_categoryToPrograms;

    using ProgramCtor = Program::ProgramBase*(*)();
    QMap<QString, ProgramCtor> m_programCtors;

    // Members
    Program::ProgramBase*   m_program = Q_NULLPTR;
    QDateTime   m_startTime;
    QTimer      m_upTimer;
    qint64      m_upHour = 0;

    // Stats
    QPushButton*    m_btnStatsEdit = Q_NULLPTR;
    QPushButton*    m_btnStatsReset = Q_NULLPTR;
    QLabel*         m_labelStats = Q_NULLPTR;
    QList<Stat*>    m_stats;
};

#endif // PROGRAMMANAGER_H
