#include "programmanager.h"

#include "Helpers/jsonhelper.h"
#include "Managers/keyboardmanager.h"
#include "Managers/managercollection.h"
#include "Programs/System/customcommand.h"

void ProgramManager::Initialize(Ui::MainWindow *ui)
{
    m_logManager = ManagerCollection::GetManager<LogManager>();

    m_programCategory = ui->CB_ProgramCategory;
    m_programList = ui->LW_ProgramList;
    m_settingsParent = ui->SA_ProgramSetting;
    m_settingsLayout = qobject_cast<QBoxLayout*>(ui->BL_ProgramSetting->layout());
    m_btnStart = ui->PB_StartProgram;
    m_btnResetDefault = ui->PB_RestoreDefault;
    m_labelDescription = ui->L_ProgramDescription;

    // connections
    connect(m_programCategory, &QComboBox::currentTextChanged, this, &ProgramManager::OnCategoryChanged);
    connect(m_programList, &QListWidget::currentTextChanged, this, &ProgramManager::OnProgramChanged);
    connect(m_btnStart, &QPushButton::clicked, this, &ProgramManager::OnProgramStartStop);
    connect(m_btnResetDefault, &QPushButton::clicked, this, &ProgramManager::OnResetDefault);

    // register all programs
    RegisterProgram<System::CustomCommand>();

    // populate categories
    QStringList categories;
    for (auto iter = m_categoryToPrograms.cbegin(); iter != m_categoryToPrograms.cend(); ++iter)
    {
        categories.push_back(iter.key());
    }
    std::sort(categories.begin(), categories.end());
    m_programCategory->addItems(categories);

    LoadSettings();
}

bool ProgramManager::OnCloseEvent()
{
    SaveSettings();
    RemoveProgram();
    return true;
}

void ProgramManager::OnCategoryChanged(const QString &category)
{
    m_programList->clear();

    QStringList const& list = m_categoryToPrograms.value(category);
    for (QString const& name : list)
    {
        m_programList->addItem(name);
    }
}

void ProgramManager::OnProgramChanged(const QString &name)
{
    RemoveProgram();

    QString const category = m_programCategory->currentText();
    m_program = m_programCtors[category + name]();
    m_program->PopulateSettings(m_settingsLayout);
    m_program->LoadSettings();

    m_btnResetDefault->setEnabled(m_program->HasSettings());
    m_labelDescription->setText(m_program->GetDescription());

    connect(m_program, &ProgramBase::notifyCanRun, this, &ProgramManager::OnCanRunChanged);
    connect(m_program, &ProgramBase::notifyFinished, this, &ProgramManager::OnProgramFinished);

    KeyboardManager* keyboardManager = ManagerCollection::GetManager<KeyboardManager>();
    connect(m_program, &ProgramBase::notifyFinished, keyboardManager, &KeyboardManager::OnUpdateStatus);

    OnCanRunChanged(m_program->CanRun());
}

void ProgramManager::OnCanRunChanged(bool canRun)
{
    if (!canRun && m_program->IsRunning())
    {
        StopProgram();

        m_logManager->PrintLog(m_program->GetInternalName(), "Program forced stopped as Serial or Camera is turned off", LOG_Warning);
        // TODO: log file name
        m_logManager->SetCurrentLogFile("");
    }

    m_btnStart->setEnabled(canRun);
}

void ProgramManager::OnProgramStartStop()
{
    if (!m_program) return;

    bool const canRun = m_program->CanRun();
    if (m_program->IsRunning())
    {
        StopProgram();

        m_logManager->PrintLog(m_program->GetInternalName(), "Program stopped by user", LOG_Warning);
        // TODO: log file name
        m_logManager->SetCurrentLogFile("");
    }
    else if (canRun)
    {
        m_logManager->ClearLog();
        // TODO: set log file
        m_logManager->PrintLog(m_program->GetInternalName(), "Program started");

        StartProgram();
    }

    m_btnStart->setEnabled(canRun);
}

void ProgramManager::OnProgramFinished()
{
    if (m_program && m_program->IsRunning())
    {
        StopProgram();

        m_logManager->PrintLog(m_program->GetInternalName(), "Program finished!", LOG_Success);
        // TODO: log file name
        m_logManager->SetCurrentLogFile("");
    }
}

void ProgramManager::OnResetDefault()
{
    if (!m_program) return;

    QMessageBox::StandardButton resBtn = QMessageBox::Yes;
    resBtn = QMessageBox::warning(this, "Warning", "Are you sure you want to restore default settings?\nThis will wipe the current settings.", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (resBtn == QMessageBox::Yes)
    {
        m_program->ResetDefault();
    }
}

void ProgramManager::LoadSettings()
{
    QJsonObject settings = JsonHelper::ReadSetting("ProgramSettings");
    {
        QVariant category;
        if (JsonHelper::ReadValue(settings, "CurrentCategory", category))
        {
            m_programCategory->setCurrentText(category.toString());
        }
    }
    {
        QVariant program;
        if (JsonHelper::ReadValue(settings, "CurrentProgram", program))
        {
            for (int i = 0; i < m_programList->count(); i++)
            {
                if (m_programList->item(i)->text() == program.toString())
                {
                    m_programList->setCurrentRow(i);
                    break;
                }
            }
        }
    }
}

void ProgramManager::SaveSettings() const
{
    QJsonObject settings = JsonHelper::ReadSetting("ProgramSettings");
    settings.insert("CurrentCategory", m_programCategory->currentText());
    settings.insert("CurrentProgram", m_programList->currentItem()->text());

    JsonHelper::WriteSetting("ProgramSettings", settings);
}

void ProgramManager::StartProgram()
{
    if (!m_program || m_program->IsRunning() || !m_program->CanRun()) return;

    m_program->Start();
    m_btnStart->setText("Stop Program");
    m_btnResetDefault->setEnabled(false);
    m_programCategory->setEnabled(false);
    m_programList->setEnabled(false);
    m_settingsParent->setEnabled(m_program->CanEditWhileRunning());
}

void ProgramManager::StopProgram()
{
    if (!m_program || !m_program->IsRunning()) return;

    m_program->Stop();
    m_btnStart->setText("Start Program");
    m_btnResetDefault->setEnabled(m_program->HasSettings());
    m_programCategory->setEnabled(true);
    m_programList->setEnabled(true);
    m_settingsParent->setEnabled(true);
}

template<class T>
void ProgramManager::RegisterProgram()
{
    QString const category = T::GetCategory();
    QString const name = T::GetName();

    QStringList& list = m_categoryToPrograms[category];
    list.push_back(name);

    m_programCtors[category + name] = []()->ProgramBase* { return new T(); };
}

void ProgramManager::RemoveProgram()
{
    if (!m_program) return;

    if (m_program->IsRunning())
    {
        StopProgram();
    }

    m_program->SaveSettings();

    // remove settings layout
    while (m_settingsLayout->count() > 1)
    {
        delete m_settingsLayout->takeAt(0)->widget();
    }

    delete m_program;
    m_program = Q_NULLPTR;

    m_btnResetDefault->setEnabled(false);
}
