#include "audiomanager.h"
#include "Helpers/audioconversionutils.h"
#include "Helpers/jsonhelper.h"

#define AUDIO_HEIGHT 100
#define AUDIO_RAW_WAVE_SCALE 0.04

AudioManager::~AudioManager()
{
    fftwf_free(m_fftDataIn);
    fftwf_free(m_fftDataOut);
}

void AudioManager::Initialize(Ui::MainWindow *ui)
{
    m_listInput = ui->CB_AudioInput;
    m_listOutput = ui->CB_AudioOutput;
    m_listDisplay = ui->CB_AudioDisplay;
    m_volumeSlider = ui->HS_Volume;

    m_displayImage = QImage(this->size(), QImage::Format_RGB32);
    m_displayImage.fill(Qt::black);

    // Spectrogram data
    m_fftBufferData.resize(FFT_SAMPLE_COUNT * 8);
    m_fftDataIn = fftwf_alloc_complex(FFT_SAMPLE_COUNT);
    m_fftDataOut = fftwf_alloc_complex(FFT_SAMPLE_COUNT);

    // Set up global audio format
    m_audioFormat.setSampleRate(48000);
    m_audioFormat.setChannelCount(2);
    m_audioFormat.setChannelConfig(QAudioFormat::ChannelConfigStereo);
    m_audioFormat.setSampleFormat(QAudioFormat::SampleFormat::Int16);

    connect(m_listOutput, &QComboBox::currentTextChanged, this, &AudioManager::OnOutputChanged);
    connect(m_listDisplay, &QComboBox::currentIndexChanged, this, &AudioManager::OnDisplayChanged);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &AudioManager::OnVolumeChanged);
    connect(&m_devices, &QMediaDevices::audioInputsChanged, this, &AudioManager::OnRefreshInputList);
    connect(&m_devices, &QMediaDevices::audioOutputsChanged, this, &AudioManager::OnRefreshOutputList);
    connect(this, &AudioManager::notifyDraw, this, &AudioManager::OnDraw);

    OnRefreshInputList();
    OnRefreshOutputList();

    this->setFixedHeight(AUDIO_HEIGHT);
    this->hide();
}

void AudioManager::Start()
{
    Stop();
    m_listInput->setEnabled(false);

    QMutexLocker locker(&m_sinkMutex);
    m_audioSink = new QAudioSink(m_audioOutput.device(), m_audioFormat, this);
    m_audioSink->setVolume((qreal)m_volumeSlider->value() * 0.01);
    m_audioDevice = m_audioSink->start();
}

void AudioManager::Stop()
{
    m_listInput->setEnabled(true);
    ClearRawWaveData();
    ClearFFTBufferData();

    QMutexLocker locker(&m_sinkMutex);
    if (m_audioSink)
    {
        m_audioSink->stop();
        delete m_audioSink;

        m_audioSink = Q_NULLPTR;
        m_audioDevice = Q_NULLPTR;
    }
}

void AudioManager::PushAudioData(const void *samples, unsigned int count, int64_t pts)
{
    size_t const sampleSize = count * m_audioFormat.bytesPerFrame();

    // this is called from LibVLC thread, not thread safe
    QMutexLocker locker(&m_sinkMutex);
    if (m_audioDevice)
    {
        m_audioDevice->write((const char*)samples, sampleSize);
    }

    // Convert raw samples to float
    QVector<float> newData;
    AudioConversionUtils::convertSamplesToFloat(m_audioFormat, (const char*)samples, sampleSize, newData);

    // Processing
    switch (m_displayType)
    {
    case AudioDisplayType::RawWave:
    {
        WriteRawWaveData(newData);
        break;
    }
    case AudioDisplayType::FreqBars:
    case AudioDisplayType::Spectrogram:
    {
        WriteFFTBufferData(newData);
        break;
    }
    default: break;
    }
}

void AudioManager::LoadSettings()
{
    QJsonObject settings = JsonHelper::ReadSetting("AudioSettings");
    {
        QVariant inputName;
        if (JsonHelper::ReadValue(settings, "InputName", inputName) && !inputName.toString().isEmpty())
        {
            m_listInput->setCurrentText(inputName.toString());
        }

        QVariant outputName;
        if (JsonHelper::ReadValue(settings, "OutputName", outputName) && !outputName.toString().isEmpty())
        {
            m_listOutput->setCurrentText(outputName.toString());
        }

        QVariant displayType;
        if (JsonHelper::ReadValue(settings, "DisplayType", displayType) && !displayType.toString().isEmpty())
        {
            m_listDisplay->setCurrentText(displayType.toString());
        }

        QVariant volume;
        if (JsonHelper::ReadValue(settings, "Volume", volume))
        {
            m_volumeSlider->setValue(volume.toInt());
        }
    }
}

void AudioManager::SaveSettings() const
{
    QJsonObject settings;
    settings.insert("InputName", m_listInput->currentText());
    settings.insert("OutputName", m_listOutput->currentText());
    settings.insert("DisplayType", m_listDisplay->currentText());
    settings.insert("Volume", m_volumeSlider->value());

    JsonHelper::WriteSetting("AudioSettings", settings);
}

void AudioManager::paintEvent(QPaintEvent *event)
{
    int const width = this->width();
    int const height = this->height();
    float const heightHalf = height * 0.5f;

    QPainter painter(this);
    painter.fillRect(this->rect(), Qt::black);

    QMutexLocker locker(&m_displayMutex);
    switch (m_displayType)
    {
    case AudioDisplayType::RawWave:
    {
        painter.setPen(QColor(Qt::cyan));

        // If no samples, draw a null sound line
        if (m_rawWaveData.isEmpty())
        {
            // Draw null sound line
            QPainter imagePainter(&m_displayImage);
            imagePainter.fillRect(this->rect(), Qt::black);
            imagePainter.setPen(QColor(Qt::cyan));
            imagePainter.drawLine(0, int(heightHalf + 0.5f), this->width(), int(heightHalf + 0.5f));
            painter.drawImage(this->rect(), m_displayImage);
            break;
        }

        QPoint lastPointPos(0, int(heightHalf + 0.5f));

        // Shift previously drawn wave data
        int const drawWidth = int(float(m_rawWaveData.size()) * AUDIO_RAW_WAVE_SCALE);
        m_displayImage = m_displayImage.copy(drawWidth, 0, width, height);
        QPainter imagePainter(&m_displayImage);
        imagePainter.setPen(QColor(Qt::cyan));

        // Draw the new data at the right side end of the image
        int const xPosStart = width - drawWidth;
        for (int i = 0; i < drawWidth; i++)
        {
            int sampleIndex = int(float(i) / AUDIO_RAW_WAVE_SCALE);
            if (sampleIndex >= m_rawWaveData.size()) break;

            float const p = m_rawWaveData[sampleIndex] * heightHalf + heightHalf;
            if (i == 0)
            {
                lastPointPos = QPoint(xPosStart, int(p + 0.5f));
            }
            else
            {
                QPoint newPointPos = QPoint(xPosStart + i, int(p + 0.5f));
                imagePainter.drawLine(lastPointPos, newPointPos);
                lastPointPos = newPointPos;
            }
        }

        // Finally draw the image on widget
        painter.drawImage(this->rect(), m_displayImage);
        break;
    }
    case AudioDisplayType::FreqBars:
    {
        if (m_spectrogramData.empty())
        {
            break;
        }

        // only draw the latest data
        QVector<float> const& spectrogramData = m_spectrogramData.back();

        float const freqRes = float(m_audioFormat.sampleRate()) / FFT_SAMPLE_COUNT;
        int const indexStart = int(float(m_freqLow) / freqRes);
        int const indexEnd = int(float(m_freqHigh) / freqRes) + 1;
        int const drawWidth = indexEnd - indexStart;

        if (drawWidth >= width)
        {
            // More samples then width, will need to ignore some
            float const sampleRatio = float(drawWidth) / float(width);
            for (int i = 0; i < width; i++)
            {
                int sampleIndex = indexStart + int(sampleRatio * i);
                if (sampleIndex >= spectrogramData.size()) break;

                float const logMag = spectrogramData[sampleIndex];
                if (logMag > 0.0f)
                {
                    painter.setPen(AudioConversionUtils::getMagnitudeColor(logMag));
                    painter.drawLine(i, height, i, int((1.0f - logMag) * height));
                }
            }
        }
        else
        {
            // fewer samples than width, we need to scale it up
            float nextXPos = 0.0f;
            float const barWidth = float(width) / float(drawWidth);
            for (int i = 0; i < drawWidth; i++)
            {
                int sampleIndex = indexStart + i;
                if (sampleIndex >= spectrogramData.size()) break;

                float const logMag = spectrogramData[sampleIndex];
                if (logMag > 0.0f)
                {
                    painter.setPen(AudioConversionUtils::getMagnitudeColor(logMag));
                    painter.drawRect(int(nextXPos), int((1.0f - logMag) * height), int(barWidth), height);
                }
                nextXPos += barWidth;
            }
        }
        break;
    }
    case AudioDisplayType::Spectrogram:
    {
        for (QVector<float> const& spectrogramData : std::as_const(m_spectrogramData))
        {
            // Don't draw spectrogram if audio is not started
            if (!m_audioSink) return;

            // Shift previously drawn spectrogram data
            m_displayImage = m_displayImage.copy(1, 0, width, height);
            QPainter imagePainter(&m_displayImage);

            float const freqRes = float(m_audioFormat.sampleRate()) / FFT_SAMPLE_COUNT;
            int const indexStart = int(float(m_freqLow) / freqRes);
            int const indexEnd = int(float(m_freqHigh) / freqRes);

            float const sampleRatio = float(indexEnd - indexStart + 1) / float(height);
            for (int i = 0; i < height; i++)
            {
                int sampleIndex = indexStart + int(sampleRatio * i);
                if (sampleIndex >= spectrogramData.size()) break;

                imagePainter.setPen(AudioConversionUtils::getMagnitudeColor(spectrogramData[sampleIndex]));
                imagePainter.drawPoint(width - 1, i);
            }

            // Finally draw the image on widget
            painter.drawImage(this->rect(), m_displayImage);
        }
        break;
    }
    default: break;
    }
}

void AudioManager::resizeEvent(QResizeEvent *event)
{
    // shift image to right side
    m_displayImage = m_displayImage.copy(m_displayImage.width() - this->width(), 0, this->width(), this->height());
}

void AudioManager::OnRefreshInputList()
{
    QString const previousInput = m_listInput->currentText();

    m_listInput->clear();
    m_listInput->addItem("None");

    bool foundPreviousInput = previousInput == "None";
    for (const QAudioDevice &device : QMediaDevices::audioInputs())
    {
        m_listInput->addItem(device.description());
        if (!foundPreviousInput && device.description() == previousInput)
        {
            foundPreviousInput = true;
        }
    }

    // keep the previous input
    if (foundPreviousInput)
    {
        m_listInput->setCurrentText(previousInput);
    }
}

void AudioManager::OnRefreshOutputList()
{
    QString const previousOutput = m_listOutput->currentText();

    m_listOutput->clear();
    m_listOutput->addItem("Default");

    bool foundPreviousOutput = previousOutput == "Default";
    for (const QAudioDevice &device : QMediaDevices::audioOutputs())
    {
        m_listOutput->addItem(device.description());
        if (!foundPreviousOutput && device.description() == previousOutput)
        {
            foundPreviousOutput = true;
        }
    }

    // keep the previous output
    if (foundPreviousOutput)
    {
        m_listInput->setCurrentText(previousOutput);
    }
}

void AudioManager::OnOutputChanged(QString const& str)
{
    for (const QAudioDevice &device : QMediaDevices::audioOutputs())
    {
        if (device.description() == str || (device.isDefault() && m_listOutput->currentText() == "Default"))
        {
            m_audioOutput.setDevice(device);

            // device changed, may have to start audio again
            if (m_audioSink)
            {
                Start();
            }
            return;
        }
    }
}

void AudioManager::OnDisplayChanged(int index)
{
    m_displayType = (AudioDisplayType)index;
    ClearRawWaveData();
    ClearFFTBufferData();

    this->update();
}

void AudioManager::OnVolumeChanged(int value)
{
    qreal const norm = (qreal)value * 0.01;
    if (m_audioSink)
    {
        m_audioSink->setVolume(norm);
    }
}

void AudioManager::OnDraw()
{
    this->update();
}

void AudioManager::WriteRawWaveData(const QVector<float> &newData)
{
    QMutexLocker locker(&m_displayMutex);

    int const size = newData.size() / 2;
    if (m_rawWaveData.size() != size)
    {
        m_rawWaveData.resize(size);
    }

    // Average LR channels
    for (int i = 0; i < size; i++)
    {
        m_rawWaveData[i] = (newData[2*i] + newData[2*i+1]) * 0.5f;
    }

    emit notifyDraw();
}

void AudioManager::ClearRawWaveData()
{
    for (float& f : m_rawWaveData)
    {
        f = 0.0f;
    }
}

void AudioManager::WriteFFTBufferData(const QVector<float> &newData)
{
    QMutexLocker locker(&m_displayMutex);

    // Push new data to buffer
    int const frameCount = newData.size() / 2;
    for (int i = 0; i < frameCount && i < m_fftBufferData.size() - 1; i++)
    {
        m_fftBufferData[m_fftNewDataStart] = (newData[2*i] + newData[2*i+1]) * 0.5f;

        // Warp back to beginning of the buffer
        m_fftNewDataStart++;
        if (m_fftNewDataStart >= m_fftBufferData.size())
        {
            m_fftNewDataStart = 0;
        }
    }

    // Check if we have enough data
    int unprocessedDataSize = m_fftNewDataStart - m_fftAnalysisStart;
    if (m_fftNewDataStart < m_fftAnalysisStart)
    {
        unprocessedDataSize += m_fftBufferData.size();
    }

    if (unprocessedDataSize >= FFT_SAMPLE_COUNT)
    {
        // if we have more data ready, do multiple FFT a frame
        int const fftCount = unprocessedDataSize / FFT_SAMPLE_COUNT;
        if (m_spectrogramData.size() != fftCount)
        {
            m_spectrogramData.resize(fftCount);
        }

        for (QVector<float>& spectrogramData : m_spectrogramData)
        {
            // Grab input FFT data, apply Hanning window to reduce leakage
            QVector<float> const& hanningFunction = AudioConversionUtils::getHanningFunction();
            int pos = m_fftAnalysisStart;
            for (int i = 0; i < FFT_SAMPLE_COUNT; i++)
            {
                m_fftDataIn[i][REAL] = m_fftBufferData[pos] * hanningFunction[i];
                m_fftDataIn[i][IMAG] = 0.0f;

                pos++;
                if (pos >= m_fftBufferData.size())
                {
                    pos = 0;
                }
            }

            // Shift to the next window
            m_fftAnalysisStart = (m_fftAnalysisStart + FFT_WINDOW_STEP) % m_fftBufferData.size();

            AudioConversionUtils::fft(FFT_SAMPLE_COUNT, m_fftDataIn, m_fftDataOut);
            AudioConversionUtils::fftOutToSpectrogram(FFT_SAMPLE_COUNT, m_fftDataOut, spectrogramData);
        }
    }
    else
    {
        qDebug() << "Waiting for more FFT input data";
    }

    emit notifyDraw();
}

void AudioManager::ClearFFTBufferData()
{
    QMutexLocker locker(&m_displayMutex);

    m_fftNewDataStart = 0;
    m_fftAnalysisStart = 0;
    for (float& f : m_fftBufferData)
    {
        f = 0.0f;
    }
    for (QVector<float>& spectrogramData : m_spectrogramData)
    {
        for (float& f : spectrogramData)
        {
            f = 0.0f;
        }
    }
    for (int i = 0; i < FFT_SAMPLE_COUNT; i++)
    {
        m_fftDataIn[i][REAL] = 0.0f;
        m_fftDataIn[i][IMAG] = 0.0f;
        m_fftDataOut[i][REAL] = 0.0f;
        m_fftDataOut[i][IMAG] = 0.0f;
    }

    m_displayImage.fill(Qt::black);
}
