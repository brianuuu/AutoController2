#include "programmanager.h"

#include "Helpers/jsonhelper.h"
#include "Programs/System/customcommand.h"

void ProgramManager::Initialize(Ui::MainWindow *ui)
{
    m_programCategory = ui->CB_ProgramCategory;
    m_programList = ui->LW_ProgramList;
    m_programSettings = qobject_cast<QBoxLayout*>(ui->SA_ProgramSetting->layout());
    m_btnResetDefault = ui->PB_RestoreDefault;
    m_labelDescription = ui->L_ProgramDescription;

    // connections
    connect(m_programCategory, &QComboBox::currentTextChanged, this, &ProgramManager::OnCategoryChanged);
    connect(m_programList, &QListWidget::currentTextChanged, this, &ProgramManager::OnProgramChanged);
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
    m_program->PopulateSettings(m_programSettings);
    m_program->LoadSettings();

    m_btnResetDefault->setEnabled(m_program->HasSettings());
    m_labelDescription->setText(m_program->GetDescription());
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

    m_program->SaveSettings();

    // remove settings layout
    while (m_programSettings->count() > 1)
    {
        delete m_programSettings->takeAt(0)->widget();
    }

    delete m_program;
    m_program = Q_NULLPTR;

    m_btnResetDefault->setEnabled(false);
}
