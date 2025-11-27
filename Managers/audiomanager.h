#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QAudioDevice>
#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioSink>
#include <QComboBox>
#include <QMediaDevices>
#include <QMutex>
#include <QSlider>
#include <QWidget>

class AudioManager : public QWidget
{
    Q_OBJECT

public:
    AudioManager
    (
        QComboBox* listInput,
        QComboBox* listOutput,
        QComboBox* listDisplay,
        QSlider* volumeSlider,
        QWidget* parent = nullptr
    );

    QAudioFormat const GetAudioFormat() const { return m_audioFormat; }
    QString GetDeviceName() const;

    void Start();
    void Stop();

    void PushAudioData(const void *samples, unsigned int count, int64_t pts);

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
    void OnInputChanged(QString const& str);
    void OnOutputChanged(QString const& str);

private:
    // UI
    QComboBox*  m_listInput = Q_NULLPTR;
    QComboBox*  m_listOutput = Q_NULLPTR;
    QComboBox*  m_listDisplay = Q_NULLPTR;
    QSlider*    m_volumeSlider = Q_NULLPTR;

    QMediaDevices   m_devices;
    QAudioFormat    m_audioFormat;
    QAudioInput     m_audioInput;
    QAudioOutput    m_audioOutput;

    QMutex          m_mutex;
    QAudioSink*     m_audioSink = Q_NULLPTR;
    QIODevice*      m_audioDevice = Q_NULLPTR;
};

#endif // AUDIOMANAGER_H
