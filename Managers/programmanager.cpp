#include "programmanager.h"

#include "Programs/System/customcommand.h"

void ProgramManager::Initialize(Ui::MainWindow *ui)
{
    m_programCategory = ui->CB_ProgramCategory;
    m_programList = ui->LW_ProgramList;
    m_programSettings = qobject_cast<QBoxLayout*>(ui->SA_ProgramSetting->layout());

    // connections
    connect(m_programCategory, &QComboBox::currentTextChanged, this, &ProgramManager::OnCategoryChanged);
    connect(m_programList, &QListWidget::currentTextChanged, this, &ProgramManager::OnProgramChanged);

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
    if (m_program)
    {
        m_program->SaveSettings();

        // remove settings layout
        while (m_programSettings->count() > 1)
        {
            delete m_programSettings->takeAt(0)->widget();
        }

        delete m_program;
        m_program = Q_NULLPTR;
    }

    QString const category = m_programCategory->currentText();
    m_program = m_programCtors[category + name]();
    m_program->PopulateSettings(m_programSettings);
}

void ProgramManager::LoadSettings()
{

}

void ProgramManager::SaveSettings() const
{

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
