#include "programbase.h"

#include "Helpers/jsonhelper.h"
#include "Helpers/ocrentrydatabase.h"
#include "Managers/audiomanager.h"
#include "Managers/discordmanager.h"
#include "Managers/logmanager.h"
#include "Managers/programmanager.h"
#include "Managers/profilemanager.h"
#include "Managers/serialmanager.h"
#include "Managers/videomanager.h"
#include "Managers/vlcmanager.h"
#include "defines.h"

namespace Program
{
ProgramBase::ProgramBase(QObject *parent) : ModuleHolder(parent)
{
    m_programManager = ManagerCollection::GetManager<ProgramManager>();
    m_profileManager = ManagerCollection::GetManager<ProfileManager>();
    m_discordManager = ManagerCollection::GetManager<DiscordManager>();
    m_serialManager = ManagerCollection::GetManager<SerialManager>();
    m_audioManager = ManagerCollection::GetManager<AudioManager>();
    m_videoManager = ManagerCollection::GetManager<VideoManager>();
    m_vlcManager = ManagerCollection::GetManager<VlcManager>();

    connect(m_serialManager->GetHolder(), &SerialHolder::notifySerialStatus, this, &ProgramBase::OnCanRunChanged);
    connect(m_audioManager->GetInputList(), &QComboBox::currentTextChanged, this, &ProgramBase::OnCanRunChanged);
    connect(m_audioManager, &AudioManager::notifySoundDetected, this, &ProgramBase::OnSoundDetected);
    connect(m_vlcManager, &VlcManager::notifyHasVideo, this, &ProgramBase::OnCanRunChanged);

    m_timer.setSingleShot(true);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &ProgramBase::OnWaitTimeout);

    LogManager* logManager = ManagerCollection::GetManager<LogManager>();
    connect(this, &ProgramBase::notifyLog, logManager, &LogManager::PrintLog);
}

ProgramBase::~ProgramBase()
{
    ClearModules();
}

void ProgramBase::LoadSettings()
{
    QJsonObject allSettings = JsonHelper::ReadSetting("ProgramSettings");
    QJsonObject settings = JsonHelper::ReadObject(allSettings, GetInternalName());

    QVariant hasRun;
    if (JsonHelper::ReadValue(settings, "HasRun", hasRun) && hasRun.toBool())
    {
        m_hasRun = true;
    }

    for (Setting::SettingBase* setting : std::as_const(m_savedSettings))
    {
        setting->Load(settings);
    }
}

void ProgramBase::SaveSettings() const
{
    QJsonObject settings;
    settings.insert("HasRun", m_hasRun);
    for (Setting::SettingBase* setting : m_savedSettings)
    {
        setting->Save(settings);
    }

    QJsonObject allSettings = JsonHelper::ReadSetting("ProgramSettings");
    allSettings.insert(GetInternalName(), settings);

    JsonHelper::WriteSetting("ProgramSettings", allSettings);
}

void ProgramBase::PopulateSettings(QBoxLayout *layout)
{
    AddText(layout, "This program has no settings.", false, true);
    AddSpacer(layout);
}

bool ProgramBase::CanRun() const
{
    return ValidSerial() && ValidVideo() && ValidAudio();
}

void ProgramBase::ResetDefault()
{
    for (Setting::SettingBase* setting : std::as_const(m_savedSettings))
    {
        setting->ResetDefault();
    }
}

void ProgramBase::Start()
{
    m_started = true;
    m_elapsedTimer.invalidate();

    Module::ModuleBase::ResetNextID();
    CleanOCRFiles();

    if (RequireAudio())
    {
        m_audioManager->ToggleSpectrogram(true);
    }

    emit notifyStarted();
}

void ProgramBase::Stop()
{
    m_audioManager->StopDetection();
    m_audioManager->ToggleSpectrogram(false);
    m_timer.stop();

    ClearModules();
    m_started = false;
}

bool ProgramBase::ValidSerial() const
{
    return !RequireSerial() || m_serialManager->IsConnected();
}

bool ProgramBase::ValidVideo() const
{
    return !RequireVideo() || m_vlcManager->HasVideo();
}

bool ProgramBase::ValidAudio() const
{
    return !RequireAudio() || (m_audioManager->GetDeviceName() != "None" && m_vlcManager->HasVideo());
}

void ProgramBase::OnCanRunChanged()
{
    emit notifyCanRun(CanRun());
}

void ProgramBase::OnModuleFinishQuit()
{
    if (!m_started) return;

    // finish program if this module is finished
    Module::ModuleBase* module = qobject_cast<Module::ModuleBase*>(sender());
    if (!module) return;

    int const result = module->GetResult();
    emit notifyFinished(result == 0);
}

bool ProgramBase::OnModuleErrorQuit()
{
    if (!m_started) return true;

    // finish program if this module is errored out
    Module::ModuleBase* module = qobject_cast<Module::ModuleBase*>(sender());
    if (!module) return true;

    // module was already deleted
    if (!m_modules.contains(module)) return true;

    int const result = module->GetResult();
    if (result < 0)
    {
        emit notifyFinished(false);
        return true;
    }

    return false;
}

void ProgramBase::RegisterStat(Stat &stat, const QString &name, bool hideZero)
{
    stat.SetName(name);
    stat.SetHideZero(hideZero);
    m_programManager->RegisterStat(stat);
}

void ProgramBase::PrintLog(const QString &log, LogType type) const
{
    emit notifyLog(GetInternalName(), log, type);
}

void ProgramBase::UnhandedStateRunCommand()
{
    emit notifyFinished(false, "Unhandled state after command is finished");
}

void ProgramBase::UnhandedStateFrameCapture()
{
    emit notifyFinished(false, "Unhandled state after frame capture has result");
}

bool ProgramBase::EnsureOCRDatabase(const QString &database)
{
    LanguageType const language = m_profileManager->GetLanguageType();
    if (!OCREntryDatabase::EnsureDatabase(database, language))
    {
        PrintLog(OCREntryDatabase::GetNoDatabaseError(database, language), LOG_Error);
        emit notifyFinished(false);
        return false;
    }
    else if (!ProfileManager::OCRTrainedDataExist(language))
    {
        PrintLog(ProfileManager::GetNoTrainedDataError(language), LOG_Error);
        emit notifyFinished(false);
        return false;
    }
    else
    {
        PrintLog(database + " database cached for language: " + LanguageToString(m_profileManager->GetLanguageType()), LOG_Important);
        return true;
    }
}

void ProgramBase::SendDiscordMessage(const QString &title, bool isMention, bool dmOnly, bool hasImage, LogType type, const QList<Discord::EmbedField> &fields)
{
    if (!m_discordManager->IsEnabled()) return;

    Discord::Embed embed = m_discordManager->GetEmbedTemplate(title);
    embed.setColor(LogTypeToColor(type).rgb() & 0xFFFFFF);

    // add custom fields
    for (Discord::EmbedField const& field : fields)
    {
        embed.addField(field);
    }

    // append program stats
    QString fieldMsg = m_programManager->GetCurrentCategory() + ": " + m_programManager->GetCurrentProgram();
    fieldMsg += "\n Up Time: " + m_programManager->GetUpTimeString();

    QString const statsString = m_programManager->GetStatsString();
    if (statsString != "N/A")
    {
        fieldMsg += "\n" + statsString;
    }
    embed.addField(Discord::EmbedField("Program Stats", fieldMsg, false));

    // get image
    QImage frame;
    if (hasImage)
    {
        frame = m_videoManager->GetFrameData();
    }

    // send message
    m_discordManager->SendMessage(embed, isMention, dmOnly, hasImage ? &frame : Q_NULLPTR);
}

QLabel *ProgramBase::AddText(QBoxLayout *layout, const QString &str, bool isBold, bool isBig)
{
    QLabel* label = new QLabel(str);
    QFont font = label->font();
    font.setBold(isBold);
    if (isBig)
    {
        font.setPointSize(font.pointSize() + 4);
    }
    label->setFont(font);
    layout->addWidget(label);

    return label;
}

QLabel *ProgramBase::AddText(QBoxLayout *layout, const QString &str, bool isBold, QColor color, bool isBig)
{
    QLabel* label = AddText(layout, str, isBold);
    QPalette palette = label->palette();
    palette.setColor(QPalette::WindowText, color);
    label->setPalette(palette);

    return label;
}

void ProgramBase::AddSetting
(
    QBoxLayout *layout,
    const QString &name,
    const QString &description,
    QWidget *setting,
    bool isHorizontal
)
{
    AddSettings(layout, name, description, {setting}, isHorizontal);
}

void ProgramBase::AddSettings
(
    QBoxLayout *layout,
    const QString &name,
    const QString &description,
    QList<QWidget*> settings,
    bool isHorizontal
)
{
    for (QWidget* setting : settings)
    {
        setting->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    QWidget* widget = new QWidget();
    layout->addWidget(widget);

    QVBoxLayout* vBoxLayout = new QVBoxLayout();
    {
        if (!name.isEmpty())
        {
            QLabel* labelName = new QLabel(name);
            QFont font = labelName->font();
            font.setBold(true);
            labelName->setFont(font);
            labelName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            vBoxLayout->addWidget(labelName);
        }

        if (!description.isEmpty())
        {
            QLabel* labelDescription = new QLabel(description);
            labelDescription->setWordWrap(true);
            vBoxLayout->addWidget(labelDescription);
        }

        vBoxLayout->setContentsMargins(0,0,0,0);
        vBoxLayout->setSpacing(0);

        if (!isHorizontal)
        {
            for (QWidget* setting : settings)
            {
                vBoxLayout->addWidget(setting);
            }
            widget->setLayout(vBoxLayout);
        }
    }

    if (isHorizontal)
    {
        QHBoxLayout* hBoxLayout = new QHBoxLayout(widget);
        hBoxLayout->addLayout(vBoxLayout);
        for (QWidget* setting : settings)
        {
            hBoxLayout->addWidget(setting);
        }
        hBoxLayout->setContentsMargins(0,0,0,0);
    }
}

void ProgramBase::AddSeparator(QBoxLayout *layout)
{
    QFrame* frame = new QFrame();
    frame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    frame->setFrameShape(QFrame::HLine);
    frame->setFrameShadow(QFrame::Sunken);
    layout->addWidget(frame);
}

void ProgramBase::AddSpacer(QBoxLayout *layout)
{
    QWidget* widget = new QWidget();
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(widget);
}

void ProgramBase::CleanOCRFiles()
{
    QStringList const files = QDir(OCR_PATH).entryList({"*.png", "*.txt"}, QDir::Files);
    for (QString const& file : files)
    {
        QFile::remove(OCR_PATH + file);
    }
}

}
