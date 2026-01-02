#include "profilemanager.h"

#include "../ui_mainwindow.h"
#include "Helpers/jsonhelper.h"
#include "Managers/managercollection.h"
#include "Managers/audiomanager.h"
#include "Programs/programbase.h"
#include "defines.h"

void ProfileManager::Initialize(Ui::MainWindow *ui)
{
    connect(ui->PB_ProfileSettings, &QPushButton::clicked, this, &ProfileManager::OnShow);

    this->setWindowTitle("Global Settings");
    this->resize(500,200);

    // setup layout
    QVBoxLayout* vBoxLayout = new QVBoxLayout(this);
    QScrollArea* scrollArea = new QScrollArea();
    QWidget* scrollWidget = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(scrollWidget);
    vBoxLayout->setContentsMargins(0,0,0,0);
    vBoxLayout->addWidget(scrollArea);

    // system settings
    Program::ProgramBase::AddText(scrollLayout, "System Settings", true);
    {
        QGroupBox* group = new QGroupBox();
        group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        scrollLayout->addWidget(group);
        QVBoxLayout* layout = new QVBoxLayout(group);

        m_language = new Setting::SettingLanguage("Language");
        Program::ProgramBase::AddSetting(layout, "Language:", "Language of the Nintendo Switch system or the current game. Required for OCR (Text Recognition)", m_language, true);
        connect(m_language, &QComboBox::currentIndexChanged, this, &ProfileManager::OnLanguageChanged);

        m_system = new Setting::SettingSystem("System");
        Program::ProgramBase::AddSetting(layout, "System:", "Type of the current Nintendo Switch system. This can affect what command to be used", m_system, true);
    }

    // program settings
    Program::ProgramBase::AddText(scrollLayout, "Program Settings", true);
    {
        QGroupBox* group = new QGroupBox();
        group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        scrollLayout->addWidget(group);
        QVBoxLayout* layout = new QVBoxLayout(group);

        m_playSound = new Setting::SettingCheckBox("PlaySound", "", true);
        m_btnPlaySound = new QPushButton("Play Sound");
        Program::ProgramBase::AddSettings(layout, "Play Sound at Program Finish:", "A sound will be played when a program finishes", {m_playSound, m_btnPlaySound}, true);
        m_playSound->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
        connect(m_playSound, &QCheckBox::checkStateChanged, this, &ProfileManager::OnPlaySoundChecked);
        connect(m_btnPlaySound, &QPushButton::clicked, this, [this]{ PlaySound(); });

        m_customSoundEnabled = new Setting::SettingCheckBox("CustomSoundEnabled", "", false);
        m_customSoundPath = new Setting::SettingLineEdit("CustomSoundPath");
        m_customSoundPath->setReadOnly(true);
        m_customSoundPath->setEnabled(false);
        m_btnCustomSound = new QToolButton();
        m_btnCustomSound->setText("...");
        m_btnCustomSound->setEnabled(false);
        Program::ProgramBase::AddSettings(layout, "Custom Sound:", "Play this custom sound instead of the default one", {m_customSoundEnabled, m_customSoundPath, m_btnCustomSound}, true);
        m_customSoundEnabled->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
        m_btnCustomSound->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        connect(m_customSoundEnabled, &QCheckBox::checkStateChanged, this, &ProfileManager::OnCustomSoundChecked);
        connect(m_customSoundPath, &QLineEdit::textChanged, this, &ProfileManager::OnCustomSoundChanged);
        connect(m_btnCustomSound, &QToolButton::clicked, this, &ProfileManager::OnCustomSoundClicked);

        m_playSoundSuppress = new Setting::SettingSpinBox("SoundSuppress", 0, INT_MAX, 1);
        Program::ProgramBase::AddSetting(layout, "Sound Suppression:", "Prevent sound from playing if program duration is less than this many minutes", m_playSoundSuppress, true);
    }

    LoadSettings();
}

bool ProfileManager::OnCloseEvent()
{
    // triggers when main window closes
    SaveSettings();
    this->hide();
    return true;
}

LanguageType ProfileManager::GetLanguageType() const
{
    return (LanguageType)m_language->currentIndex();
}

SystemType ProfileManager::GetSystemType() const
{
    return (SystemType)m_system->currentIndex();
}

void ProfileManager::PlaySound(quint64 minutes)
{
    if (!m_playSound->isChecked() || minutes < m_playSoundSuppress->value()) return;

    if (m_mediaPlayer)
    {
        m_mediaPlayer->stop();
        delete m_mediaPlayer;
    }

    AudioManager* audioManager = ManagerCollection::GetManager<AudioManager>();
    m_mediaPlayer = new QMediaPlayer(this);
    m_mediaPlayer->setAudioOutput(audioManager->GetAudioOutput());

    if (m_customSoundEnabled->isChecked() && !m_customSoundPath->text().isEmpty())
    {
        m_mediaPlayer->setSource(QUrl::fromLocalFile(m_customSoundPath->text()));
    }
    else
    {
        m_mediaPlayer->setSource(QUrl::fromLocalFile(RESOURCES_PATH + "PokemonBW/06-caught-a-pokemon.mp3"));
    }

    m_mediaPlayer->play();
}

bool ProfileManager::OCRTrainedDataExist(LanguageType type)
{
    QString const prefix = LanguageToPrefix(type);
    return QFile::exists(OCR_PATH + prefix + ".traineddata");
}

QString ProfileManager::GetNoTrainedDataError(LanguageType type)
{
    QString const languageName = LanguageToString(type);
    return "Language trained data for '" + languageName + "' for Tesseract is missing, please goto 'Resources/Tesseract' folder and follow the instructions in README.md";
}

void ProfileManager::closeEvent(QCloseEvent *event)
{
    SaveSettings();
    QWidget::closeEvent(event);
}

void ProfileManager::OnShow()
{
    this->show();
    if (this->isMinimized())
    {
        this->showNormal();
    }
    this->activateWindow();
}

void ProfileManager::OnLanguageChanged(int index)
{
    LanguageType const type = (LanguageType)index;
    if (!OCRTrainedDataExist(GetLanguageType()))
    {
        QMessageBox::warning(this, "Warning", GetNoTrainedDataError(type), QMessageBox::Ok);
    }
}

void ProfileManager::OnPlaySoundChecked(Qt::CheckState state)
{
    bool const enabled = state == Qt::Checked;
    m_btnPlaySound->setEnabled(enabled);
}

void ProfileManager::OnCustomSoundChecked(Qt::CheckState state)
{
    bool const enabled = state == Qt::Checked;
    m_customSoundPath->setEnabled(enabled);
    m_btnCustomSound->setEnabled(enabled);
}

void ProfileManager::OnCustomSoundChanged(const QString &file)
{
    // Save directory
    QFileInfo info(file);
    m_path = info.dir().absolutePath();
}

void ProfileManager::OnCustomSoundClicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select Custom Sound"), m_path, "Sounds (*.mp3 *.wav)");
    if (file == Q_NULLPTR) return;
    m_customSoundPath->setText(file);
}

void ProfileManager::LoadSettings()
{
    QJsonObject profileSettings = JsonHelper::ReadSetting("ProfileSettings");
    {
        QJsonObject system = JsonHelper::ReadObject(profileSettings, "System");
        m_language->Load(system);
        m_system->Load(system);

        QJsonObject program = JsonHelper::ReadObject(profileSettings, "Program");
        m_playSound->Load(program);
        m_customSoundEnabled->Load(program);
        m_customSoundPath->Load(program);
        m_playSoundSuppress->Load(program);
    }
    {
        QJsonObject windowSize = JsonHelper::ReadObject(profileSettings, "WindowSize");
        QVariant x, y;
        if (JsonHelper::ReadValue(windowSize, "X", x) && JsonHelper::ReadValue(windowSize, "Y", y))
        {
            this->move(x.toInt(), y.toInt());
        }

        QVariant width, height;
        if (JsonHelper::ReadValue(windowSize, "Width", width) && JsonHelper::ReadValue(windowSize, "Height", height))
        {
            this->resize(width.toInt(), height.toInt());
        }
    }
}

void ProfileManager::SaveSettings() const
{
    QJsonObject system;
    m_language->Save(system);
    m_system->Save(system);

    QJsonObject program;
    m_playSound->Save(program);
    m_customSoundEnabled->Save(program);
    m_customSoundPath->Save(program);
    m_playSoundSuppress->Save(program);

    QJsonObject windowSize;
    windowSize.insert("Width", this->width());
    windowSize.insert("Height", this->height());
    windowSize.insert("X", this->pos().x());
    windowSize.insert("Y", this->pos().y());

    QJsonObject profileSettings;
    profileSettings.insert("System", system);
    profileSettings.insert("Program", program);
    profileSettings.insert("WindowSize", windowSize);

    JsonHelper::WriteSetting("ProfileSettings", profileSettings);
}
