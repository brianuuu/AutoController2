#include "devsounddetection.h"
#include "Managers/audiomanager.h"
#include "defines.h"

namespace Program::Development
{

DevSoundDetection::DevSoundDetection(QObject *parent)
    : ProgramBase{parent}
{
}

void DevSoundDetection::PopulateSettings(QBoxLayout *layout)
{
    m_btnPlay = new QPushButton("Play");
    m_btnPlay->setEnabled(false);

    m_file = new Setting::SettingLineEdit("WavFile", "PokemonLA/ShinySFX");
    m_savedSettings.insert(m_file);
    AddSettings(layout, "Sound File:", "File must be located in Resources folder and .wav format", {m_btnPlay, m_file}, true);
    connect(m_btnPlay, &QPushButton::clicked, this, &DevSoundDetection::OnPlaySound);
    connect(m_file, &QLineEdit::textChanged, this, &DevSoundDetection::OnFileChanged);
    m_btnPlay->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    m_minScore = new Setting::SettingDoubleSpinBox("MinScore", 0.0, 10.0, 0.19);
    m_minScore->setDecimals(3);
    m_savedSettings.insert(m_minScore);
    AddSetting(layout, "Min Score:", "", m_minScore, true);

    m_lowPassFilter = new Setting::SettingSpinBox("LowPassFilter", 0, 19000, 5000);
    m_lowPassFilter->setSingleStep(100);
    m_savedSettings.insert(m_lowPassFilter);
    AddSetting(layout, "Low Pass Filter:", "", m_lowPassFilter, true);

    AddSpacer(layout);

    OnFileChanged();
}

bool DevSoundDetection::CanRun() const
{
    return ProgramBase::CanRun() && m_validSound;
}

void DevSoundDetection::Start()
{
    ProgramBase::Start();

    m_soundID = m_audioManager->AddDetection(m_file->text(), m_minScore->value(), m_lowPassFilter->value());
    if (m_soundID == 0)
    {
        emit notifyFinished(false);
    }
    else
    {
        m_audioManager->StartDetection(m_soundID);
        m_audioManager->SetShowMaxScore(true);
    }
}

void DevSoundDetection::Stop()
{
    m_audioManager->SetShowMaxScore(false);
    ProgramBase::Stop();
}

void DevSoundDetection::OnFileChanged()
{
    m_validSound = QFile::exists(GetFileName());
    m_btnPlay->setEnabled(m_validSound);
    OnCanRunChanged();
}

void DevSoundDetection::OnPlaySound()
{
    if (m_mediaPlayer)
    {
        m_mediaPlayer->stop();
        delete m_mediaPlayer;
    }

    m_mediaPlayer = new QMediaPlayer(this);
    m_mediaPlayer->setAudioOutput(m_audioManager->GetAudioOutput());
    m_mediaPlayer->setSource(QUrl::fromLocalFile(GetFileName()));
    m_mediaPlayer->play();
}

QString DevSoundDetection::GetFileName() const
{
    return RESOURCES_PATH + m_file->text() + ".wav";
}

}
