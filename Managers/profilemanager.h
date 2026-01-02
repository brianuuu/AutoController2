#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QFileDialog>
#include <QGroupBox>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QToolButton>
#include <QWidget>

#include "Programs/Settings/settingcheckbox.h"
#include "Programs/Settings/settingcombobox.h"
#include "Programs/Settings/settinglanguage.h"
#include "Programs/Settings/settinglineedit.h"
#include "Programs/Settings/settingspinbox.h"
#include "Programs/Settings/settingsystem.h"
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
    void PlaySound(quint64 minutes = INT64_MAX);
    bool StreamCounterEnabled() const;
    bool StreamCounterExcludePrefix() const;

    // Utils
    static bool OCRTrainedDataExist(LanguageType type);
    static QString GetNoTrainedDataError(LanguageType type);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void OnShow();

    // System
    void OnLanguageChanged(int index);

    // Program
    void OnPlaySoundChecked(Qt::CheckState state);
    void OnCustomSoundChecked(Qt::CheckState state);
    void OnCustomSoundChanged(QString const& file);
    void OnCustomSoundClicked();

private:
    void LoadSettings();
    void SaveSettings() const;

private:
    QString m_path;

    // System
    Setting::SettingLanguage* m_language = Q_NULLPTR;
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
};

#endif // PROFILEMANAGER_H
