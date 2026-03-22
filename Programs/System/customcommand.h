#ifndef SYSTEM_CUSTOMCOMMAND_H
#define SYSTEM_CUSTOMCOMMAND_H

#include <QFileDialog>
#include <QMediaPlayer>
#include <QPushButton>
#include <QRegularExpressionValidator>

#include "../programbase.h"
#include "Settings/System/settingpreset.h"
#include "Settings/settinglineedit.h"
#include "Settings/settingtextedit.h"
#include "Types/categorytype.h"

namespace Program::System
{
class CustomCommand : public ProgramBase
{
    Q_OBJECT
public:
    explicit CustomCommand(QObject* parent = nullptr);

    static CategoryType GetCategory() { return CT_System; }
    static QString GetName() { return "Custom Command"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    QString GetInternalName() const override { return "System-CustomCommand"; }
    QString GetDescription() const override {
        return "Runs a pre-made command, or make custom commands.";
    }

    bool RequireSerial() const override { return true; }
    bool RequireVideo() const override { return false; }
    bool RequireAudio() const override { return m_sound->currentIndex() > 0; }

    bool CanRun() const override;

    void Start() override;
    void Stop() override;

private slots:
    void OnListChanged(QString const& str);
    void OnSoundChanged();
    void OnPlaySound();
    void OnCommandChanged();
    void OnCommandSave();

    void OnCommandFinished() override;
    void OnSoundDetected(int id) override;

private:
    void VerifyCommand();

private:
    Setting::System::SettingPreset* m_list = Q_NULLPTR;
    Setting::System::SettingPreset* m_sound = Q_NULLPTR;
    Setting::SettingLineEdit* m_command = Q_NULLPTR;
    Setting::SettingTextEdit* m_description = Q_NULLPTR;
    QLabel* m_labelStatus = Q_NULLPTR;
    QPushButton* m_btnPlay = Q_NULLPTR;
    QPushButton* m_btnSave = Q_NULLPTR;
    QPushButton* m_btnDelete = Q_NULLPTR;
    QPushButton* m_btnDirectory = Q_NULLPTR;

    QMediaPlayer* m_mediaPlayer = Q_NULLPTR;
    int m_soundID = 0;
    bool m_validCommand = false;
};
}

#endif // SYSTEM_CUSTOMCOMMAND_H
