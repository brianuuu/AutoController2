#include "profilemanager.h"

#include "../ui_mainwindow.h"
#include "Helpers/jsonhelper.h"
#include "Programs/programbase.h"

void ProfileManager::Initialize(Ui::MainWindow *ui)
{
    connect(ui->PB_ProfileSettings, &QPushButton::clicked, this, &ProfileManager::OnShow);

    // setup layout
    this->setWindowTitle("Profile Settings");

    QVBoxLayout* vBoxLayout = new QVBoxLayout(this);
    vBoxLayout->setSizeConstraint(QLayout::SetMinimumSize);
    vBoxLayout->setSizeConstraint(QLayout::SetFixedSize);

    // system settings
    QGroupBox* systemGroup = new QGroupBox("System");
    vBoxLayout->addWidget(systemGroup);
    {
        QVBoxLayout* systemLayout = new QVBoxLayout(systemGroup);

        m_language = new Setting::SettingLanguage("Language");
        Program::ProgramBase::AddSetting(systemLayout, "Language:", "Language of the Nintendo Switch system or the current game", m_language, true);
        connect(m_language, &QComboBox::currentIndexChanged, this, &ProfileManager::OnLanguageChanged);
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

bool ProfileManager::OcrTrainedDataExist(LanguageType type)
{
    QString const prefix = LanguageToPrefix(type);
    return QFile::exists("../Resources/Tesseract/" + prefix + ".traineddata");
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
    LanguageType type = (LanguageType)index;
    if (!OcrTrainedDataExist(GetLanguageType()))
    {
        QString const languageName = LanguageToString(type);
        QMessageBox::warning(this, "Warning", "Language trained data for '" + languageName + "' for Tesseract is missing, please goto 'Resources/Tesseract' folder and follow the instructions in README.md.", QMessageBox::Ok);
    }
}

void ProfileManager::LoadSettings()
{
    QJsonObject profileSettings = JsonHelper::ReadSetting("ProfileSettings");
    {
        QJsonObject system = JsonHelper::ReadObject(profileSettings, "System");
        m_language->Load(system);
    }
    {
        QJsonObject windowSize = JsonHelper::ReadObject(profileSettings, "WindowSize");
        QVariant x, y;
        if (JsonHelper::ReadValue(windowSize, "X", x) && JsonHelper::ReadValue(windowSize, "Y", y))
        {
            this->move(x.toInt(), y.toInt());
        }
    }
}

void ProfileManager::SaveSettings() const
{
    QJsonObject system;
    m_language->Save(system);

    QJsonObject windowSize;
    windowSize.insert("X", this->pos().x());
    windowSize.insert("Y", this->pos().y());

    QJsonObject profileSettings;
    profileSettings.insert("System", system);
    profileSettings.insert("WindowSize", windowSize);

    JsonHelper::WriteSetting("ProfileSettings", profileSettings);
}
