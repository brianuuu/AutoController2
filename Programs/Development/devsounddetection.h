#ifndef DEVSOUNDDETECTION_H
#define DEVSOUNDDETECTION_H

#include <QFile>
#include <QMediaPlayer>
#include <QPushButton>

#include "../programbase.h"
#include "Settings/settingdoublespinbox.h"
#include "Settings/settinglineedit.h"
#include "Settings/settingspinbox.h"

namespace Program::Development
{
class DevSoundDetection : public ProgramBase
{
public:
    explicit DevSoundDetection(QObject *parent = nullptr);

    static QString GetCategory() { return "Development"; }
    static QString GetName() { return "Sound Detection"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    QString GetInternalName() const override { return "Dev-SoundDetection"; }
    QString GetDescription() const override {
        return "Test sound detection";
    }

    bool RequireSerial() const override { return false; }
    bool RequireVideo() const override { return false; }
    bool RequireAudio() const override { return true; }

    bool CanRun() const override;
    bool CanControlWhileRunning() const override { return true; }
    bool CanEditWhileRunning() const override { return false; }

    void Start() override;
    void Stop() override;

private slots:
    void OnFileChanged();
    void OnPlaySound();

private:
    QString GetFileName() const;

private:
    QMediaPlayer* m_mediaPlayer = Q_NULLPTR;
    int m_soundID = 0;
    bool m_validSound = false;

    QPushButton* m_btnPlay = Q_NULLPTR;
    Setting::SettingLineEdit* m_file = Q_NULLPTR;
    Setting::SettingDoubleSpinBox* m_minScore = Q_NULLPTR;
    Setting::SettingSpinBox* m_lowPassFilter = Q_NULLPTR;
};
}

#endif // DEVSOUNDDETECTION_H
