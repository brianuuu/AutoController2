#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QBoxLayout>
#include <QFileDialog>
#include <QGroupBox>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QToolButton>
#include <QWidget>

#include "Managers/managercollection.h"
#include "Settings/System/settinglanguage.h"
#include "Settings/System/settingthreadpriority.h"
#include "Settings/settingcheckbox.h"
#include "Settings/settingcombobox.h"
#include "Settings/settinglineedit.h"
#include "Settings/settingspinbox.h"
#include "Settings/settingsystem.h"
#include "Types/languagetype.h"
#include "Types/systemtype.h"

namespace Ui { class MainWindow; }

class ProfileManager : public QWidget
{
    Q_OBJECT
public:
    explicit ProfileManager(QWidget *parent = nullptr) : QWidget(parent) {}
    static QString GetTypeID() { return "Profile"; }
    void Initialize(Ui::MainWindow* ui);

    bool OnCloseEvent();

public:
    // System
    LanguageType GetLanguageType() const;
    SystemType GetSystemType() const;

    // Program
    QComboBox* GetStreamCounterComboBox() { return m_streamCounter; }
    void PlaySound(quint64 minutes = INT64_MAX);
    bool StreamCounterEnabled() const;
    bool StreamCounterExcludePrefix() const;

    // Performance
    QThread::Priority GetModulePriority() const { return m_modulePriority->GetPriority(); }
    QThread::Priority GetSerialPriority() const { return m_serialPriority->GetPriority(); }

    // Development
    bool GetHasDebugConsole() const { return m_debugConsole->isChecked(); }
    bool GetSwapRedBlue() const { return m_swapRedBlue->isChecked(); }
    bool GetDebugButton() const { return m_debugButton->isChecked(); }

    // Utils
    static bool OCRTrainedDataExist(LanguageType type);
    static QString GetNoTrainedDataError(LanguageType type);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void OnShow();
    void OnResetSection();

    // System
    void OnLanguageChanged(int index);

    // Program
    void OnPlaySoundChecked(Qt::CheckState state);
    void OnCustomSoundChecked(Qt::CheckState state);
    void OnCustomSoundChanged(QString const& file);
    void OnCustomSoundClicked();

private: // types
    struct Section {
        QVBoxLayout* m_layout = Q_NULLPTR;
        QPushButton* m_btnReset = Q_NULLPTR;
        QSet<Setting::SettingBase*> m_settings;
    };

private:
    void LoadSettings();
    void SaveSettings() const;

    Section& CreateSection(QVBoxLayout* parentLayout, QString const& name);
    void AddResetButtonToSection(Section& section);

private:
    QString m_path;
    DiscordManager* m_discordManager = Q_NULLPTR;
    std::map<QString, Section> m_sections;

    // System
    Setting::System::SettingLanguage* m_language = Q_NULLPTR;
    Setting::SettingSystem* m_system = Q_NULLPTR;

    // Program
    Setting::SettingCheckBox* m_playSound = Q_NULLPTR;
    QPushButton* m_btnPlaySound = Q_NULLPTR;
    Setting::SettingCheckBox* m_customSoundEnabled = Q_NULLPTR;
    Setting::SettingLineEdit* m_customSoundPath = Q_NULLPTR;
    QToolButton* m_btnCustomSound = Q_NULLPTR;
    Setting::SettingSpinBox* m_playSoundSuppress = Q_NULLPTR;
    QMediaPlayer* m_mediaPlayer = Q_NULLPTR;
    Setting::SettingComboBox* m_streamCounter = Q_NULLPTR;

    // Performance
    Setting::System::SettingThreadPriority* m_mainPriority = Q_NULLPTR;
    Setting::System::SettingThreadPriority* m_modulePriority = Q_NULLPTR;
    Setting::System::SettingThreadPriority* m_serialPriority = Q_NULLPTR;

    // Development
    Setting::SettingCheckBox* m_debugConsole = Q_NULLPTR;
    Setting::SettingCheckBox* m_swapRedBlue = Q_NULLPTR;
    Setting::SettingCheckBox* m_debugButton = Q_NULLPTR;
};

#endif // PROFILEMANAGER_H
