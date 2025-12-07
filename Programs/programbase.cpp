#include "programbase.h"
#include "Helpers/jsonhelper.h"

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

void ProgramBase::ResetDefault()
{
    for (SettingBase* setting : m_settings)
    {
        setting->ResetDefault();
    }
}

void ProgramBase::AddSingleItem
(
    QBoxLayout *layout,
    const QString &name,
    const QString &description,
    QWidget *setting
)
{
    setting->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QWidget* widget = new QWidget();
    layout->insertWidget(layout->count() - 1, widget);

    QVBoxLayout* vBoxLayout = new QVBoxLayout();
    {
        QLabel* labelName = new QLabel(name);
        QFont font = labelName->font();
        font.setBold(true);
        labelName->setFont(font);
        labelName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        QLabel* labelDescription = new QLabel(description);
        labelDescription->setWordWrap(true);

        vBoxLayout->addWidget(labelName);
        vBoxLayout->addWidget(labelDescription);
        vBoxLayout->setContentsMargins(0,0,0,0);
        vBoxLayout->setSpacing(0);
    }

    QHBoxLayout* hBoxLayout = new QHBoxLayout(widget);
    hBoxLayout->addLayout(vBoxLayout);
    hBoxLayout->addWidget(setting);
    hBoxLayout->setContentsMargins(0,0,0,0);
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
    AddSingleItem(layout, name, description, comboBox);
    return comboBox;
}
