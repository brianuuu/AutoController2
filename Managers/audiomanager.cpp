#include "audiomanager.h"

AudioManager::AudioManager
(
    QWidget* parent,
    QComboBox* listInput,
    QComboBox* listOutput,
    QComboBox* listDisplay,
    QSlider* volumeSlider
)
    : m_listInput(listInput)
    , m_listOutput(listOutput)
    , m_listDisplay(listDisplay)
    , m_volumeSlider(volumeSlider)
{
    // Set up global audio format
    m_audioFormat.setSampleRate(48000);
    m_audioFormat.setChannelCount(2);
    m_audioFormat.setChannelConfig(QAudioFormat::ChannelConfigStereo);
    m_audioFormat.setSampleFormat(QAudioFormat::SampleFormat::Float);

    connect(m_listInput, &QComboBox::currentTextChanged, this, &AudioManager::OnInputChanged);
    connect(m_listOutput, &QComboBox::currentTextChanged, this, &AudioManager::OnOutputChanged);
    connect(&m_devices, &QMediaDevices::audioInputsChanged, this, &AudioManager::OnRefreshInputList);
    connect(&m_devices, &QMediaDevices::audioOutputsChanged, this, &AudioManager::OnRefreshOutputList);

    OnRefreshInputList();
    OnRefreshOutputList();
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

    bool foundPreviousOutput = false;
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
        if (device.description() == str)
        {
            m_audioOutput.setDevice(device);
            return;
        }
    }
}
