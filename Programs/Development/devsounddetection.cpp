#include "devsounddetection.h"
#include "Helpers/jsonhelper.h"
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
    QDir const directory(AudioManager::GetDirectory());
    QStringList const files = directory.entryList({"*" + AudioManager::GetExtension()}, QDir::Files);

    QStringList names = { CUSTOM_SELECTION };
    for (QString const& file : files)
    {
        names << file.mid(0, file.size() - AudioManager::GetExtension().size());
    }

    m_list = new Setting::SettingComboBox("Preset", names);
    m_savedSettings.insert(m_list);
    AddSetting(layout, "Preset Select:", "Select sound detection preset", m_list, true);
    connect(m_list, &QComboBox::currentTextChanged, this, &DevSoundDetection::OnListChanged);

    m_btnPlay = new QPushButton("Play");
    m_btnPlay->setEnabled(false);

    m_file = new Setting::SettingLineEdit("WavFile", "PokemonLA/ShinySFX");
    AddSettings(layout, "Sound File:", "File must be located in Resources folder and .wav format", {m_btnPlay, m_file}, true);
    connect(m_btnPlay, &QPushButton::clicked, this, &DevSoundDetection::OnPlaySound);
    connect(m_file, &QLineEdit::textChanged, this, &DevSoundDetection::OnFileChanged);
    connect(m_file, &QLineEdit::textEdited, this, &DevSoundDetection::OnFileEdited);
    m_btnPlay->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    m_minScore = new Setting::SettingDoubleSpinBox("MinScore", 0.0, 10.0, 0.5);
    m_minScore->setDecimals(3);
    AddSetting(layout, "Min Score:", "", m_minScore, true);

    m_lowFreqFilter = new Setting::SettingSpinBox("LowFreqFilter", 0, 19000, 1000);
    m_lowFreqFilter->setSingleStep(100);
    AddSetting(layout, "Low Frequency Filter:", "", m_lowFreqFilter, true);

    m_btnSave = new QPushButton("Save As...");
    m_btnDelete = new QPushButton("Delete");
    m_btnDirectory = new QPushButton("Open Directory");
    AddSettings(layout, "", "", {m_btnSave, m_btnDelete, m_btnDirectory}, true);
    connect(m_btnSave, &QPushButton::clicked, this, &DevSoundDetection::OnSave);
    connect(m_btnDelete, &QPushButton::clicked, this, &DevSoundDetection::OnDelete);
    connect(m_btnDirectory, &QPushButton::clicked, this, &DevSoundDetection::OnOpenDirectory);

    AddSpacer(layout);

    // set initial text
    OnListChanged(m_list->currentText());
}

bool DevSoundDetection::CanRun() const
{
    return ProgramBase::CanRun() && m_validSound;
}

void DevSoundDetection::Start()
{
    ProgramBase::Start();

    m_soundID = m_audioManager->AddDetection(m_file->text(), m_minScore->value(), m_lowFreqFilter->value());
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
    // user input or programmatic change
    m_validSound = QFile::exists(GetFileName());
    m_btnPlay->setEnabled(m_validSound);
    m_btnSave->setEnabled(m_validSound);
    OnCanRunChanged();
}

void DevSoundDetection::OnFileEdited()
{
    // user input only
    m_list->setCurrentText(CUSTOM_SELECTION);
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

void DevSoundDetection::OnListChanged(const QString &str)
{
    m_btnDelete->setEnabled(!m_started && m_list->currentText() != CUSTOM_SELECTION);
    if (str == CUSTOM_SELECTION)
    {
        // should save
        m_savedSettings.insert(m_file);
        m_savedSettings.insert(m_minScore);
        m_savedSettings.insert(m_lowFreqFilter);
        return;
    }
    else
    {
        // don't save
        m_savedSettings.remove(m_file);
        m_savedSettings.remove(m_minScore);
        m_savedSettings.remove(m_lowFreqFilter);
    }

    QString const name = AudioManager::GetDirectory() + str + AudioManager::GetExtension();
    QJsonObject const object = JsonHelper::ReadJson(name);

    QVariant value;
    if (JsonHelper::ReadValue(object, "File", value))
    {
        m_file->setText(value.toString());
    }

    if (JsonHelper::ReadValue(object, "MinScore", value))
    {
        m_minScore->setValue(value.toDouble());
    }

    if (JsonHelper::ReadValue(object, "LowFreqFilter", value))
    {
        m_lowFreqFilter->setValue(value.toInt());
    }
}

void DevSoundDetection::OnSave()
{
    QString const file = QFileDialog::getSaveFileName(m_btnSave, tr("Save Preset As"), AudioManager::GetDirectory(), "Sound Detection (*" + AudioManager::GetExtension() + ")");
    if (file == Q_NULLPTR) return;

    QFileInfo const info(file);
    QString name = info.fileName();
    name = name.mid(0, name.size() - AudioManager::GetExtension().size());

    if (name == CUSTOM_SELECTION)
    {
        QMessageBox::critical(m_list, "Error", "This name is not allowed", QMessageBox::Ok);
        return;
    }

    QJsonObject object;
    object.insert("File", m_file->text());
    object.insert("MinScore", m_minScore->value());
    object.insert("LowFreqFilter", m_lowFreqFilter->value());
    JsonHelper::WriteJson(file, object);

    if (m_list->findText(name) == -1)
    {
        m_list->addItem(name);
        m_list->model()->sort(0);
    }

    m_list->setCurrentText(name);
}

void DevSoundDetection::OnDelete()
{
    QMessageBox::StandardButton resBtn = QMessageBox::Yes;
    resBtn = QMessageBox::warning(m_btnDelete, "Warning", "Are you sure you want to delete current preset?\nThis cannot be undone.", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (resBtn == QMessageBox::Yes)
    {
        QFile::remove(AudioManager::GetDirectory() + m_list->currentText() + AudioManager::GetExtension());
        m_list->removeItem(m_list->currentIndex());
    }
}

void DevSoundDetection::OnOpenDirectory()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(AudioManager::GetDirectory()));
}

QString DevSoundDetection::GetFileName() const
{
    return RESOURCES_PATH + m_file->text() + ".wav";
}

}
