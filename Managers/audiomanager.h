#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QAudioDevice>
#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioSink>
#include <QComboBox>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMediaDevices>
#include <QMutex>
#include <QSlider>
#include <QWidget>

#include <fftw3.h>

#include "Helpers/audiofileholder.h"
#include "Managers/managercollection.h"

namespace Ui { class MainWindow; }

class AudioManager : public QWidget
{
    Q_OBJECT

public:
    explicit AudioManager(QWidget* parent = nullptr) : QWidget(parent) {}
    ~AudioManager();
    static QString GetTypeID() { return "Audio"; }
    void Initialize(Ui::MainWindow* ui);

    QAudioFormat const GetAudioFormat() const { return m_audioFormat; }
    QString GetDeviceName() const { return m_listInput->currentText(); }
    QComboBox* GetInputList() const { return m_listInput; }
    QAudioOutput* GetAudioOutput() { return &m_audioOutput; }

    void Start();
    void Stop();

    void PushAudioData(const void *samples, unsigned int count, int64_t pts);

    void LoadSettings();
    void SaveSettings() const;

    // Sound detection
    void ToggleSpectrogram(bool enabled);
    int AddDetection(QString const& fileName, float minScore, int lowFreqFilter);
    void StartDetection(int id);
    void StopDetection(int id = 0);
    bool HasDetection(int id);
    void DoDetection();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent *event) override;

signals:
    void notifyDraw();
    void notifyFFTBufferData();
    void notifySoundDetected(int id);

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
    void OnDisplayChanged(int index);
    void OnVolumeChanged(int value);
    void OnDraw();

private:
    void StartAudioSink();
    void ClearAudioSink();

    // Raw Wave
    void WriteRawWaveData(QVector<float> const& newData);
    void ClearRawWaveData();

    // Spectrogram
    void WriteFFTBufferData(QVector<float> const& newData);
    void ProcessFFTBufferData();
    void ClearFFTBufferData();

private:
    LogManager* m_logManager = Q_NULLPTR;

    // UI
    QComboBox*  m_listInput = Q_NULLPTR;
    QComboBox*  m_listOutput = Q_NULLPTR;
    QComboBox*  m_listDisplay = Q_NULLPTR;
    QSlider*    m_volumeSlider = Q_NULLPTR;

    // Devices
    QMediaDevices       m_devices;
    QAudioFormat        m_audioFormat;
    QAudioOutput        m_audioOutput;

    // Display
    QMutex              m_displayMutex;
    QImage              m_displayImage;
    AudioDisplayType    m_displayType = AudioDisplayType::None;

    // Raw Wave data
    QVector<float>      m_rawWaveData;

    // Spectrogram data
    QVector<float>      m_fftBufferData;
    int                 m_fftNewDataStart = 0;
    int                 m_fftAnalysisStart = 0;
    int                 m_freqLow = 0;
    int                 m_freqHigh = 10000;
    fftwf_complex*      m_fftDataIn;
    fftwf_complex*      m_fftDataOut;
    QVector<QVector<float>> m_spectrogramData;

    // Output
    QMutex          m_sinkMutex;
    QAudioSink*     m_audioSink = Q_NULLPTR;
    QIODevice*      m_audioDevice = Q_NULLPTR;

    // Sound detection
    QMap<QString, AudioFileHolder*>     m_audioFileHolders;
    QSet<AudioFileHolder*>              m_detectingSounds;
    QVector<SpikeIDScore>               m_cachedSpikes;
    int                                 m_detectedWindowSize = 0;
};

#endif // AUDIOMANAGER_H
