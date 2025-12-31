#include "programbase.h"

#include "Helpers/jsonhelper.h"
#include "Helpers/ocrentrydatabase.h"
#include "Managers/audiomanager.h"
#include "Managers/logmanager.h"
#include "Managers/profilemanager.h"
#include "Managers/serialmanager.h"
#include "Managers/vlcmanager.h"
#include "defines.h"

namespace Program
{
ProgramBase::ProgramBase(QObject *parent) : QObject(parent)
{
    m_profileManager = ManagerCollection::GetManager<ProfileManager>();
    m_serialManager = ManagerCollection::GetManager<SerialManager>();
    m_audioManager = ManagerCollection::GetManager<AudioManager>();
    m_vlcManager = ManagerCollection::GetManager<VlcManager>();

    connect(m_serialManager->GetHolder(), &SerialHolder::notifySerialStatus, this, &ProgramBase::OnCanRunChanged);
    connect(m_audioManager->GetInputList(), &QComboBox::currentTextChanged, this, &ProgramBase::OnCanRunChanged);
    connect(m_vlcManager, &VlcManager::notifyHasVideo, this, &ProgramBase::OnCanRunChanged);

    m_timer.setSingleShot(true);
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
    for (Setting::SettingBase* setting : std::as_const(m_savedSettings))
    {
        setting->Load(settings);
    }
}

void ProgramBase::SaveSettings() const
{
    QJsonObject settings;
    for (Setting::SettingBase* setting : m_savedSettings)
    {
        setting->Save(settings);
    }

    QJsonObject allSettings = JsonHelper::ReadSetting("ProgramSettings");
    allSettings.insert(GetInternalName(), settings);

    JsonHelper::WriteSetting("ProgramSettings", allSettings);
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

    Module::ModuleBase::ResetNextID();
    CleanOCRFiles();

    emit notifyStarted();
}

void ProgramBase::Stop()
{
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
    return !RequireAudio() || m_audioManager->GetDeviceName() != "None";
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
    emit notifyFinished(result);
}

bool ProgramBase::OnModuleErrorQuit()
{
    if (!m_started) return true;

    // finish program if this module is errored out
    Module::ModuleBase* module = qobject_cast<Module::ModuleBase*>(sender());
    if (!module) return false;

    int const result = module->GetResult();
    if (result < 0)
    {
        emit notifyFinished(result);
        return true;
    }

    return false;
}

void ProgramBase::PrintLog(const QString &log, LogType type) const
{
    emit notifyLog(GetInternalName(), log, type);
}

void ProgramBase::AddModule(Module::ModuleBase *module)
{
    if (!module) return;

    m_modules.insert(module);
    module->moveToThread(module);
    module->start();
}

void ProgramBase::ClearModule(QObject *sender)
{
    Module::ModuleBase* module = qobject_cast<Module::ModuleBase*>(sender);
    ClearModule(module);
}

void ProgramBase::ClearModule(Module::ModuleBase *module)
{
    if (!module || !m_modules.contains(module)) return;

    module->stop();
    module->wait();
    delete module;

    m_modules.remove(module);
}

void ProgramBase::ClearModules()
{
    for (Module::ModuleBase* module : std::as_const(m_modules))
    {
        module->stop();
        module->wait();
        delete module;
    }

    m_modules.clear();
}

bool ProgramBase::EnsureOCRDatabase(const QString &database)
{
    if (!OCREntryDatabase::EnsureDatabase(database, m_profileManager->GetLanguageType()))
    {
        PrintLog(OCREntryDatabase::GetNoDatabaseError(database, m_profileManager->GetLanguageType()), LOG_Error);
        emit notifyFinished(-1);
        return false;
    }
    else
    {
        PrintLog(database + " database cached for language: " + LanguageToString(m_profileManager->GetLanguageType()), LOG_Important);
        return true;
    }
}

QLabel *ProgramBase::AddText(QBoxLayout *layout, const QString &str, bool isBold)
{
    QLabel* label = new QLabel(str);
    QFont font = label->font();
    font.setBold(isBold);
    label->setFont(font);
    layout->addWidget(label);

    return label;
}

QLabel *ProgramBase::AddText(QBoxLayout *layout, const QString &str, bool isBold, QColor color)
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
