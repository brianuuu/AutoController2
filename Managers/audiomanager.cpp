#include "audiomanager.h"

AudioManager::AudioManager
(
    QComboBox* listInput,
    QComboBox* listOutput,
    QComboBox* listDisplay,
    QSlider* volumeSlider,
    QWidget* parent
)
    : QWidget(parent)
    , m_listInput(listInput)
    , m_listOutput(listOutput)
    , m_listDisplay(listDisplay)
    , m_volumeSlider(volumeSlider)
{
    // Set up global audio format
    m_audioFormat.setSampleRate(48000);
    m_audioFormat.setChannelCount(2);
    m_audioFormat.setChannelConfig(QAudioFormat::ChannelConfigStereo);
    m_audioFormat.setSampleFormat(QAudioFormat::SampleFormat::Int16);

    connect(m_listInput, &QComboBox::currentTextChanged, this, &AudioManager::OnInputChanged);
    connect(m_listOutput, &QComboBox::currentTextChanged, this, &AudioManager::OnOutputChanged);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &AudioManager::OnVolumeChanged);
    connect(&m_devices, &QMediaDevices::audioInputsChanged, this, &AudioManager::OnRefreshInputList);
    connect(&m_devices, &QMediaDevices::audioOutputsChanged, this, &AudioManager::OnRefreshOutputList);

    OnRefreshInputList();
    OnRefreshOutputList();
}

QString AudioManager::GetDeviceName() const
{
    return m_listInput->currentText();
}

void AudioManager::Start()
{
    Stop();
    m_listInput->setEnabled(false);

    m_mutex.lock();
    {
        m_audioSink = new QAudioSink(m_audioOutput.device(), m_audioFormat, this);
        m_audioSink->setVolume((qreal)m_volumeSlider->value() * 0.01);
        m_audioDevice = m_audioSink->start();
    }
    m_mutex.unlock();
}

void AudioManager::Stop()
{
    m_mutex.lock();
    {
        if (m_audioSink)
        {
            m_audioSink->stop();
            delete m_audioSink;

            m_audioSink = Q_NULLPTR;
            m_audioDevice = Q_NULLPTR;
        }
    }
    m_mutex.unlock();

    m_listInput->setEnabled(true);
}

void AudioManager::PushAudioData(const void *samples, unsigned int count, int64_t pts)
{
    size_t const sampleSize = count * m_audioFormat.bytesPerFrame();

    // this is called from LibVLC thread, not thread safe
    if (m_mutex.tryLock())
    {
        if (m_audioDevice)
        {
            m_audioDevice->write((const char*)samples, sampleSize);
        }
        m_mutex.unlock();
    }
}

void AudioManager::OnRefreshInputList()
{
    QString const previousInput = m_listInput->currentText();

    m_listInput->clear();

    bool foundPreviousInput = false;
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

void AudioManager::OnInputChanged(QString const& str)
{
    for (const QAudioDevice &device : QMediaDevices::audioInputs())
    {
        if (device.description() == str)
        {
            m_audioInput.setDevice(device);
            return;
        }
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

void AudioManager::OnVolumeChanged(int value)
{
    qreal const norm = (qreal)value * 0.01;
    if (m_audioSink)
    {
        m_audioSink->setVolume(norm);
    }
}
