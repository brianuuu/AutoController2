#include "programbase.h"

#include "Helpers/jsonhelper.h"
#include "Managers/audiomanager.h"
#include "Managers/logmanager.h"
#include "Managers/serialmanager.h"
#include "Managers/vlcmanager.h"

ProgramBase::ProgramBase(QObject *parent) : QObject(parent)
{
    m_logManager = ManagerCollection::GetManager<LogManager>();
    m_serialManager = ManagerCollection::GetManager<SerialManager>();
    m_audioManager = ManagerCollection::GetManager<AudioManager>();
    m_vlcManager = ManagerCollection::GetManager<VlcManager>();

    connect(m_serialManager, &SerialManager::notifySerialStatus, this, &ProgramBase::OnCanRunChanged);
    connect(m_audioManager->GetInputList(), &QComboBox::currentTextChanged, this, &ProgramBase::OnCanRunChanged);
    connect(m_vlcManager, &VlcManager::notifyHasVideo, this, &ProgramBase::OnCanRunChanged);
}

void ProgramBase::LoadSettings()
{
    QJsonObject allSettings = JsonHelper::ReadSetting("ProgramSettings");
    QJsonObject settings = JsonHelper::ReadObject(allSettings, GetInternalName());
    for (SettingBase* setting : std::as_const(m_settings))
    {
        setting->Load(settings);
    }
}

void ProgramBase::SaveSettings() const
{
    QJsonObject settings;
    for (SettingBase* setting : m_settings)
    {
        setting->Save(settings);
    }

    QJsonObject allSettings = JsonHelper::ReadSetting("ProgramSettings");
    allSettings.insert(GetInternalName(), settings);

    JsonHelper::WriteSetting("ProgramSettings", allSettings);
}

bool ProgramBase::CanRun() const
{
    return (!RequireSerial() || m_serialManager->IsConnected())
        && (!RequireVideo() || m_vlcManager->HasVideo())
        && (!RequireAudio() || m_audioManager->GetDeviceName() != "None");
}

void ProgramBase::ResetDefault()
{
    for (SettingBase* setting : std::as_const(m_settings))
    {
        setting->ResetDefault();
    }
}

void ProgramBase::Start()
{
    m_started = true;
}

void ProgramBase::Stop()
{
    m_started = false;
}

void ProgramBase::OnCanRunChanged()
{
    emit notifyCanRun(CanRun());
}

void ProgramBase::PrintLog(const QString &log)
{
     m_logManager->PrintLog(GetInternalName(), log);
}

void ProgramBase::AddSingleItem(QBoxLayout *layout, QWidget *widget)
{
    layout->insertWidget(layout->count() - 1, widget);
}

QLabel *ProgramBase::AddSingleText(QBoxLayout *layout, const QString &str, bool isBold)
{
    QLabel* label = new QLabel(str);
    QFont font = label->font();
    font.setBold(isBold);
    label->setFont(font);
    AddSingleItem(layout, label);

    return label;
}

void ProgramBase::AddSingleSetting
(
    QBoxLayout *layout,
    const QString &name,
    const QString &description,
    QWidget *setting,
    bool isHorizontal
)
{
    setting->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QWidget* widget = new QWidget();
    layout->insertWidget(layout->count() - 1, widget);

    QVBoxLayout* vBoxLayout = new QVBoxLayout();
    {
        if (!name.isEmpty())
        {
            QLabel* labelName = new QLabel(name);
            QFont font = labelName->font();
            font.setBold(true);
            labelName->setFont(font);
            labelName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
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
            vBoxLayout->addWidget(setting);
            widget->setLayout(vBoxLayout);
        }
    }

    if (isHorizontal)
    {
        QHBoxLayout* hBoxLayout = new QHBoxLayout(widget);
        hBoxLayout->addLayout(vBoxLayout);
        hBoxLayout->addWidget(setting);
        hBoxLayout->setContentsMargins(0,0,0,0);
    }
}

SettingComboBox *ProgramBase::AddComboBox
(
    QBoxLayout *layout,
    const QString &name,
    const QString &description,
    const QString &settingName,
    const QStringList &list
)
{
    SettingComboBox* comboBox = new SettingComboBox(settingName, list);
    m_settings.push_back(comboBox);
    AddSingleSetting(layout, name, description, comboBox, true);
    return comboBox;
}

SettingLineEdit *ProgramBase::AddLineEdit
(
    QBoxLayout *layout,
    const QString &name,
    const QString &description,
    const QString &settingName
)
{
    SettingLineEdit* lineEdit = new SettingLineEdit(settingName);
    m_settings.push_back(lineEdit);
    AddSingleSetting(layout, name, description, lineEdit, false);
    return lineEdit;
}

SettingTextEdit *ProgramBase::AddTextEdit
(
    QBoxLayout *layout,
    const QString &name,
    const QString &description,
    const QString &settingName
)
{
    SettingTextEdit* textEdit = new SettingTextEdit(settingName);
    m_settings.push_back(textEdit);
    AddSingleSetting(layout, name, description, textEdit, false);
    return textEdit;
}
