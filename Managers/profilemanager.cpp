#include "profilemanager.h"

#include "../ui_mainwindow.h"
#include "Helpers/jsonhelper.h"
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
    QGroupBox* systemGroup = new QGroupBox();
    systemGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollLayout->addWidget(systemGroup);
    {
        QVBoxLayout* systemLayout = new QVBoxLayout(systemGroup);

        m_language = new Setting::SettingLanguage("Language");
        Program::ProgramBase::AddSetting(systemLayout, "Language:", "Language of the Nintendo Switch system or the current game. Required for OCR (Text Recognition)", m_language, true);
        connect(m_language, &QComboBox::currentIndexChanged, this, &ProfileManager::OnLanguageChanged);

        m_system = new Setting::SettingSystem("System");
        Program::ProgramBase::AddSetting(systemLayout, "System:", "Type of the current Nintendo Switch system. This can affect what command to be used", m_system, true);
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

bool ProfileManager::OCRTrainedDataExist(LanguageType type)
{
    QString const prefix = LanguageToPrefix(type);
    return QFile::exists(OCR_PATH + prefix + ".traineddata");
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
    if (!OCRTrainedDataExist(GetLanguageType()))
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
        m_system->Load(system);
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

    QJsonObject windowSize;
    windowSize.insert("Width", this->width());
    windowSize.insert("Height", this->height());
    windowSize.insert("X", this->pos().x());
    windowSize.insert("Y", this->pos().y());

    QJsonObject profileSettings;
    profileSettings.insert("System", system);
    profileSettings.insert("WindowSize", windowSize);

    JsonHelper::WriteSetting("ProfileSettings", profileSettings);
}
