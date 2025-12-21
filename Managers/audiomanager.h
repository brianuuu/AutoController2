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
#include "../ui_mainwindow.h"

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

    void Start();
    void Stop();

    void PushAudioData(const void *samples, unsigned int count, int64_t pts);

    void LoadSettings();
    void SaveSettings() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent *event) override;

signals:
    void notifyDraw();

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
    void OnOutputChanged(QString const& str);
    void OnDisplayChanged(int index);
    void OnVolumeChanged(int value);
    void OnDraw();

private:
    void WriteRawWaveData(QVector<float> const& newData);
    void ClearRawWaveData();

    // Spectrogram
    void WriteFFTBufferData(QVector<float> const& newData);
    void ClearFFTBufferData();

private:
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
};

#endif // AUDIOMANAGER_H
