#include "programbase.h"

#include "Helpers/jsonhelper.h"
#include "Managers/audiomanager.h"
#include "Managers/logmanager.h"
#include "Managers/serialmanager.h"
#include "Managers/vlcmanager.h"

namespace Program
{
ProgramBase::ProgramBase(QObject *parent) : QObject(parent)
{
    m_serialManager = ManagerCollection::GetManager<SerialManager>();
    m_audioManager = ManagerCollection::GetManager<AudioManager>();
    m_vlcManager = ManagerCollection::GetManager<VlcManager>();

    connect(m_serialManager->GetHolder(), &SerialHolder::notifySerialStatus, this, &ProgramBase::OnCanRunChanged);
    connect(m_audioManager->GetInputList(), &QComboBox::currentTextChanged, this, &ProgramBase::OnCanRunChanged);
    connect(m_vlcManager, &VlcManager::notifyHasVideo, this, &ProgramBase::OnCanRunChanged);

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

void ProgramBase::PrintLog(const QString &log, LogType type) const
{
    emit notifyLog(GetInternalName(), log, type);
}

void ProgramBase::ClearModule(Module::ModuleBase *&module)
{
    if (!module) return;

    module->stop();
    module->wait();
    delete module;

    m_modules.remove(module);
    module = Q_NULLPTR;
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

QLabel *ProgramBase::AddText(QBoxLayout *layout, const QString &str, bool isBold)
{
    QLabel* label = new QLabel(str);
    QFont font = label->font();
    font.setBold(isBold);
    label->setFont(font);
    layout->addWidget(label);

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

void ProgramBase::AddSpacer(QBoxLayout *layout)
{
    QWidget* widget = new QWidget();
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(widget);
}

}
