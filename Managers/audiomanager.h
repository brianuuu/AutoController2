#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QAudioDevice>
#include <QComboBox>
#include <QMediaDevices>
#include <QSlider>
#include <QWidget>

class AudioManager : public QWidget
{
    Q_OBJECT

public:
    AudioManager
    (
        QWidget* parent,
        QComboBox* listInput,
        QComboBox* listOutput,
        QComboBox* listDisplay,
        QSlider* volumeSlider
    );

private: // types
    enum class AudioDisplayType
    {
        None,
        RawWave,
        FreqBars,
        Spectrogram,
        COUNT
    };

private slots:
    // UI
    void OnRefreshInputList();
    void OnRefreshOutputList();

private:
    // UI
    QComboBox*  m_listInput = Q_NULLPTR;
    QComboBox*  m_listOutput = Q_NULLPTR;
    QComboBox*  m_listDisplay = Q_NULLPTR;
    QSlider*    m_volumeSlider = Q_NULLPTR;

    QMediaDevices   m_devices;
    QAudioFormat    m_audioFormat;
};

#endif // AUDIOMANAGER_H
