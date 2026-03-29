#include "profilemanager.h"

#include "../ui_mainwindow.h"
#include "Helpers/jsonhelper.h"
#include "Managers/managercollection.h"
#include "Managers/audiomanager.h"
#include "Managers/discordmanager.h"
#include "Programs/programbase.h"
#include "defines.h"

void ProfileManager::Initialize(Ui::MainWindow *ui)
{
    m_discordManager = ManagerCollection::GetManager<DiscordManager>();

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
    {
        Section& section = CreateSection(scrollLayout, "System");

        m_language = new Setting::System::SettingLanguage("Language");
        Program::ProgramBase::AddSetting(section.m_layout, "Language:", "Language of the Nintendo Switch system or the current game. Required for OCR (Text Recognition)", m_language, true);
        connect(m_language, &QComboBox::currentIndexChanged, this, &ProfileManager::OnLanguageChanged);

        m_system = new Setting::System::SettingSystem("System");
        Program::ProgramBase::AddSetting(section.m_layout, "System:", "Type of the current Nintendo Switch system. This can affect what command to be used", m_system, true);

        section.m_settings.insert(m_language);
        section.m_settings.insert(m_system);
        AddResetButtonToSection(section);
    }

    // program settings
    {
        Section& section = CreateSection(scrollLayout, "Program");

        m_playSound = new Setting::SettingCheckBox("PlaySound", "", true);
        m_btnPlaySound = new QPushButton("Play Sound");
        Program::ProgramBase::AddSettings(section.m_layout, "Play Sound at Program Finish:", "A sound will be played when a program finishes", {m_playSound, m_btnPlaySound}, true);
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
        Program::ProgramBase::AddSettings(section.m_layout, "Custom Sound:", "Play this custom sound instead of the default one", {m_customSoundEnabled, m_customSoundPath, m_btnCustomSound}, true);
        m_customSoundEnabled->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
        m_btnCustomSound->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        connect(m_customSoundEnabled, &QCheckBox::checkStateChanged, this, &ProfileManager::OnCustomSoundChecked);
        connect(m_customSoundPath, &QLineEdit::textChanged, this, &ProfileManager::OnCustomSoundChanged);
        connect(m_btnCustomSound, &QToolButton::clicked, this, &ProfileManager::OnCustomSoundClicked);

        m_playSoundSuppress = new Setting::SettingSpinBox("SoundSuppress", 0, INT_MAX, 1);
        Program::ProgramBase::AddSetting(section.m_layout, "Sound Suppression:", "Prevent sound from playing if program duration is less than this many minutes", m_playSoundSuppress, true);

        m_streamCounter = new Setting::SettingComboBox("StreamCounter", {"Disabled", "Enabled (Full Stat)", "Enabled (Numbers Only)"});
        Program::ProgramBase::AddSetting(section.m_layout, "Stream Counter:", "Program with stats will export each as individual text files to \"StreamCounters\" folder", m_streamCounter, true);

        section.m_settings.insert(m_playSound);
        section.m_settings.insert(m_customSoundEnabled);
        section.m_settings.insert(m_customSoundPath);
        section.m_settings.insert(m_playSoundSuppress);
        section.m_settings.insert(m_streamCounter);
        AddResetButtonToSection(section);
    }

    // discord settings
    {
        Section& section = CreateSection(scrollLayout, "Discord");

        m_discordManager->m_settingToken = new Setting::SettingLineEdit("BotToken");
        m_discordManager->m_settingToken->setEchoMode(QLineEdit::Password);
        Program::ProgramBase::AddSetting(section.m_layout, "Bot Token:", "Token of your discord bot, do not share this with anyone and keep it safe", m_discordManager->m_settingToken, true);

        m_discordManager->m_settingUser = new Setting::SettingLineEdit("UserID");
        m_discordManager->m_btnTestUser = new QPushButton("Send Test DM");
        m_discordManager->m_btnTestUser->setEnabled(false);
        m_discordManager->m_btnTestUser->setFixedWidth(130);
        Program::ProgramBase::AddSettings(section.m_layout, "User ID:", "The bot will send program status to this user (shiny, finished, error etc.)", {m_discordManager->m_settingUser, m_discordManager->m_btnTestUser}, true);
        m_discordManager->m_btnTestUser->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

        m_discordManager->m_settingChannel = new Setting::SettingLineEdit("ChannelID");
        m_discordManager->m_btnTestChannel = new QPushButton("Send Test Message");
        m_discordManager->m_btnTestChannel->setEnabled(false);
        m_discordManager->m_btnTestChannel->setFixedWidth(130);
        Program::ProgramBase::AddSettings(section.m_layout, "Channel ID:", "The bot will only send special program status to this channel (shiny etc.), it must have appropriate permissions in that server", {m_discordManager->m_settingChannel, m_discordManager->m_btnTestChannel}, true);
        m_discordManager->m_btnTestChannel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

        m_discordManager->m_btnStartStop = new QPushButton("Start Bot");
        m_discordManager->m_btnStartStop->setEnabled(false);
        Program::ProgramBase::AddSetting(section.m_layout, "Toggle Bot:", "Application will start bot automatically at launch if bot has started", m_discordManager->m_btnStartStop, true);

        m_discordManager->m_settingHourlyUpdate = new Setting::SettingCheckBox("HourlyUpdate", "", true);
        Program::ProgramBase::AddSetting(section.m_layout, "Hourly Status Update:", "Send Program Status message every hour to remind user a program is running", m_discordManager->m_settingHourlyUpdate, true);

        m_discordManager->m_settingFinishSuppress = new Setting::SettingSpinBox("FinishSuppress", 0, INT_MAX, 10);
        Program::ProgramBase::AddSetting(section.m_layout, "Finish Notification Suppression:", "Prevent sending program finish/error notification if program duration is less than this many minutes", m_discordManager->m_settingFinishSuppress, true);

        section.m_settings.insert(m_discordManager->m_settingToken);
        section.m_settings.insert(m_discordManager->m_settingUser);
        section.m_settings.insert(m_discordManager->m_settingChannel);
        section.m_settings.insert(m_discordManager->m_settingHourlyUpdate);
        section.m_settings.insert(m_discordManager->m_settingFinishSuppress);
        AddResetButtonToSection(section);
    }

    // performance settings
    {
        Section& section = CreateSection(scrollLayout, "Performance");

        m_mainPriority = new Setting::System::SettingThreadPriority("MainPriority", QThread::HighestPriority);
        Program::ProgramBase::AddSetting(section.m_layout, "Main Thread Priority:", "Thread priority for main GUI thread, include drawing video & audio feed (Require restart)", m_mainPriority, true);

        m_modulePriority = new Setting::System::SettingThreadPriority("ModulePriority", QThread::HighPriority);
        Program::ProgramBase::AddSetting(section.m_layout, "Module Thread Priority:", "Thread priority for modules used when running programs", m_modulePriority, true);

        m_serialPriority = new Setting::System::SettingThreadPriority("SerialPriority", QThread::NormalPriority);
        Program::ProgramBase::AddSetting(section.m_layout, "Serial Thread Priority:", "Thread priority for serial holder in charge of dispatching commands (Require restart)", m_serialPriority, true);

        section.m_settings.insert(m_mainPriority);
        section.m_settings.insert(m_modulePriority);
        section.m_settings.insert(m_serialPriority);
        AddResetButtonToSection(section);
    }

    // development settings
    {
        Section& section = CreateSection(scrollLayout, "Development");

        m_debugConsole = new Setting::SettingCheckBox("DebugConsole", "", false);
        Program::ProgramBase::AddSetting(section.m_layout, "Enable Debug Console:", "Display additional debug logs that doesn't show in output log (Require restart)", m_debugConsole, true);

        m_swapRedBlue = new Setting::SettingCheckBox("SwapRedBlue", "", false);
        Program::ProgramBase::AddSetting(section.m_layout, "Swap Red/Blue Video Channel:", "Some capture card may require swapping Red & Blue channel to display correctly (Require restarting camera)", m_swapRedBlue, true);

        m_debugButton = new Setting::SettingCheckBox("DebugButton", "", false);
        Program::ProgramBase::AddSetting(section.m_layout, "Debug Button Presses:", "Show debug log for each button press (Can get spammy)", m_debugButton, true);

        section.m_settings.insert(m_debugConsole);
        section.m_settings.insert(m_swapRedBlue);
        section.m_settings.insert(m_debugButton);
        AddResetButtonToSection(section);
    }

    m_discordManager->Initialize();
    LoadSettings();
}

ProfileManager::Section &ProfileManager::CreateSection(QVBoxLayout* parentLayout, const QString &name)
{
    Program::ProgramBase::AddText(parentLayout, name + " Settings", true, true);
    QGroupBox* group = new QGroupBox();
    group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    parentLayout->addWidget(group);

    Section& section = m_sections[name];
    section.m_layout = new QVBoxLayout(group);
    return section;
}

void ProfileManager::AddResetButtonToSection(Section &section)
{
    section.m_btnReset = new QPushButton("Reset Default Settings");
    section.m_layout->addWidget(section.m_btnReset);
    connect(section.m_btnReset, &QPushButton::clicked, this, &ProfileManager::OnResetSection);

    QFont font = section.m_btnReset->font();
    font.setPointSize(12);
    section.m_btnReset->setFont(font);
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

bool ProfileManager::StreamCounterEnabled() const
{
    return m_streamCounter->currentIndex() != 0;
}

bool ProfileManager::StreamCounterExcludePrefix() const
{
    return m_streamCounter->currentIndex() == 2;
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

void ProfileManager::OnResetSection()
{
    QWidget* widget = qobject_cast<QWidget*>(sender());
    for (auto& [name, section] : m_sections)
    {
        if (widget == section.m_btnReset)
        {
            QMessageBox::StandardButton resBtn = QMessageBox::Yes;
            resBtn = QMessageBox::warning(this, "Warning", "Are you sure you want to restore current section '" + name + "' to default settings?\nThis will wipe the current settings.", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

            if (resBtn == QMessageBox::Yes)
            {
                for (Setting::SettingBase* setting : std::as_const(section.m_settings))
                {
                    setting->ResetDefault();
                }

                if (name == "Discord" && m_discordManager->IsEnabled())
                {
                    m_discordManager->SetEnabled(false);
                }
            }
            return;
        }
    }
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

    for (auto& [name, section] : m_sections)
    {
        QJsonObject settings = JsonHelper::ReadObject(profileSettings, name);
        for (Setting::SettingBase* setting : std::as_const(section.m_settings))
        {
            setting->Load(settings);
        }

        if (name == "Discord")
        {
            QVariant enabled;
            if (JsonHelper::ReadValue(settings, "DefaultEnabled", enabled) && enabled.toBool())
            {
                m_discordManager->SetEnabled(true);
            }
        }
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
    QJsonObject profileSettings;

    for (auto& [name, section] : m_sections)
    {
        QJsonObject settings;
        for (Setting::SettingBase* setting : section.m_settings)
        {
            setting->Save(settings);
        }

        if (name == "Discord")
        {
            settings.insert("DefaultEnabled", m_discordManager->IsEnabled());
        }

        profileSettings.insert(name, settings);
    }

    QJsonObject windowSize;
    windowSize.insert("Width", this->width());
    windowSize.insert("Height", this->height());
    windowSize.insert("X", this->pos().x());
    windowSize.insert("Y", this->pos().y());
    profileSettings.insert("WindowSize", windowSize);

    JsonHelper::WriteSetting("ProfileSettings", profileSettings);
}
