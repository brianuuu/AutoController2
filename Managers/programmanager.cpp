#include "programmanager.h"

#include "../ui_mainwindow.h"
#include "Helpers/jsonhelper.h"
#include "Managers/keyboardmanager.h"
#include "Managers/logmanager.h"
#include "Managers/profilemanager.h"

#include "Programs/Development/devcommand.h"
#include "Programs/Development/devframecapture.h"
#include "Programs/PokemonLZA/donutmaker.h"
#include "Programs/System/commandrecorder.h"
#include "Programs/System/customcommand.h"

#define PROGRAM_MANUAL_PATH "../Manuals/"
#define PROGRAM_STATS_INI "../Stats.ini"
#define STREAM_COUNTER_PATH "../StreamCounters/"

void ProgramManager::Initialize(Ui::MainWindow *ui)
{
    m_logManager = ManagerCollection::GetManager<LogManager>();
    m_profileManager = ManagerCollection::GetManager<ProfileManager>();

    m_programCategory = ui->CB_ProgramCategory;
    m_programList = ui->LW_ProgramList;
    m_settingsParent = ui->BL_ProgramSetting;
    m_settingsLayout = qobject_cast<QBoxLayout*>(ui->BL_ProgramSetting->layout());
    m_btnStart = ui->PB_StartProgram;
    m_btnResetDefault = ui->PB_RestoreDefault;
    m_btnManual = ui->PB_ProgramManual;
    m_labelDescription = ui->L_ProgramDescription;
    m_labelSerial = ui->L_SerialPort;
    m_labelCamera = ui->L_CameraDevice;
    m_labelAudio = ui->L_AudioInput;
    m_labelUpTime = ui->L_UpTime;

    m_upTimer.setInterval(200);

    m_btnStatsEdit = ui->PB_StatsEdit;
    m_btnStatsReset = ui->PB_StatsReset;
    m_labelStats = ui->L_Stats;

    if (!QDir(STREAM_COUNTER_PATH).exists())
    {
        QDir().mkdir(STREAM_COUNTER_PATH);
    }

    // shortcuts
    new QShortcut(QKeySequence("F5"), this, [this]{ OnProgramStartStop(); }, Qt::ApplicationShortcut);

    // connections
    connect(m_programCategory, &QComboBox::currentTextChanged, this, &ProgramManager::OnCategoryChanged);
    connect(m_programList, &QListWidget::currentTextChanged, this, &ProgramManager::OnProgramChanged);
    connect(m_btnStart, &QPushButton::clicked, this, &ProgramManager::OnProgramStartStop);
    connect(m_btnResetDefault, &QPushButton::clicked, this, &ProgramManager::OnResetDefault);
    connect(m_btnManual, &QPushButton::clicked, this, &ProgramManager::OnManualOpen);
    connect(&m_upTimer, &QTimer::timeout, this, &ProgramManager::OnUpTimeUpdate);
    connect(m_btnStatsEdit, &QPushButton::clicked, this, &ProgramManager::OnStatsEdit);
    connect(m_btnStatsReset, &QPushButton::clicked, this, &ProgramManager::OnStatsReset);
    connect(m_profileManager->GetStreamCounterComboBox(), &QComboBox::currentIndexChanged, this, [this] { UpdateStats(); });

    // register all programs
    RegisterProgram<Program::Development::DevCommand>();
    RegisterProgram<Program::Development::DevFrameCapture>();
    RegisterProgram<Program::PokemonLZA::DonutMaker>();
    RegisterProgram<Program::System::CommandRecorder>();
    RegisterProgram<Program::System::CustomCommand>();

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

void ProgramManager::RegisterStat(int& refValue, const QString &name)
{
    if (!m_program) return;

    m_stats[&refValue] = name;

    QSettings stats(PROGRAM_STATS_INI, QSettings::IniFormat, this);
    stats.beginGroup(m_program->GetInternalName());

    // Grab value from ini, resave if ini file doesn't exist
    refValue = stats.value(name, 0).toInt();
    stats.setValue(name, refValue);

    UpdateStats();
}

void ProgramManager::IncrementStat(int &refValue, int amount)
{
    if (!m_program || !m_stats.contains(&refValue)) return;

    QSettings stats(PROGRAM_STATS_INI, QSettings::IniFormat, this);
    stats.beginGroup(m_program->GetInternalName());
    QString const name = m_stats.value(&refValue);

    // Grab value from ini, increment and save
    refValue = stats.value(name).toInt();
    refValue += amount;
    refValue = qMax(refValue, 0); // no negative, but something probably went wrong...?
    stats.setValue(name, refValue);

    UpdateStats();
}

void ProgramManager::UpdateStats(bool reset)
{
    // Delete all stream counter text files
    QDirIterator it(QString(STREAM_COUNTER_PATH));
    while (it.hasNext())
    {
        QString dir = it.next();
        QFile::remove(dir);
    }

    if (!m_program)
    {
        m_labelStats->setText("N/A");
        m_btnStatsReset->setEnabled(false);
        return;
    }

    // Grab stats for current program
    QSettings stats(PROGRAM_STATS_INI, QSettings::IniFormat, this);
    stats.beginGroup(m_program->GetInternalName());

    // Update label
    QStringList const list = stats.allKeys();
    QString statsStr;
    if (list.isEmpty())
    {
        statsStr = "N/A";
        m_btnStatsReset->setEnabled(false);
    }
    else
    {
        m_btnStatsReset->setEnabled(true);
        for (int i = 0; i < list.size(); i++)
        {
            QString const& key = list[i];
            if (i != 0)
            {
                statsStr += ", ";
            }

            if (reset)
            {
                stats.setValue(key, 0);
            }

            int count = stats.value(key, 0).toInt();
            statsStr += key + ": " + QString::number(count);

            // Write to individual files for each stat
            if (m_profileManager->StreamCounterEnabled())
            {
                QFile file(STREAM_COUNTER_PATH + key + ".txt");
                if(file.open(QIODevice::WriteOnly))
                {
                    QTextStream stream(&file);
                    if (!m_profileManager->StreamCounterExcludePrefix())
                    {
                        stream << key + ": ";
                    }
                    stream << count;
                    file.close();
                }
            }
        }
    }
    m_labelStats->setText(statsStr);
}

void ProgramManager::ClearStats()
{
    m_stats.clear();
    UpdateStats();
}

void ProgramManager::OnCategoryChanged(const QString &category)
{
    m_programList->clear();

    QStringList const& list = m_categoryToPrograms[category];
    for (QString const& name : list)
    {
        m_programList->addItem(name);
    }

    // default select 1st item
    if (m_programList->count())
    {
        m_programList->setCurrentRow(0);
    }
}

void ProgramManager::OnProgramChanged(const QString &name)
{
    RemoveProgram();
    if (name.isEmpty()) return;

    QString const category = m_programCategory->currentText();
    m_program = m_programCtors.value(category + name)();
    m_program->PopulateSettings(m_settingsLayout);
    m_program->LoadSettings();
    m_program->RegisterStats();

    m_btnManual->setEnabled(QFile::exists(PROGRAM_MANUAL_PATH + m_program->GetInternalName() + ".pdf"));
    m_btnResetDefault->setEnabled(m_program->HaveSavedSettings());
    m_labelDescription->setText(m_program->GetDescription());

    connect(m_program, &Program::ProgramBase::notifyCanRun, this, &ProgramManager::OnCanRunChanged);
    connect(m_program, &Program::ProgramBase::notifyFinished, this, &ProgramManager::OnProgramFinished);

    KeyboardManager* keyboardManager = ManagerCollection::GetManager<KeyboardManager>();
    connect(m_program, &Program::ProgramBase::notifyFinished, keyboardManager, &KeyboardManager::OnUpdateStatus);

    OnCanRunChanged(m_program->CanRun());
}

void ProgramManager::OnCanRunChanged(bool canRun)
{
    if (!m_program) return;

    if (!canRun && m_program->IsRunning())
    {
        m_logManager->PrintLog(m_program->GetInternalName(), "Program forced stopped as Serial or Camera is turned off", LOG_Warning);
        StopProgram();
    }

    auto fnSetPalette = [](QLabel* label, bool valid)
    {
        QPalette palette = label->palette();
        palette.setColor(QPalette::WindowText, valid ? QGuiApplication::palette().windowText().color() : LogTypeToColor(LOG_Error));
        label->setPalette(palette);
    };

    // highlight required elements
    fnSetPalette(m_labelSerial, m_program->ValidSerial());
    fnSetPalette(m_labelCamera, m_program->ValidVideo());
    fnSetPalette(m_labelAudio, m_program->ValidAudio());

    m_btnStart->setEnabled(canRun);
}

void ProgramManager::OnProgramStartStop()
{
    if (!m_program) return;

    bool const canRun = m_program->CanRun();
    if (m_program->IsRunning())
    {
        m_logManager->PrintLog(m_program->GetInternalName(), "Program stopped by user", LOG_Warning);
        StopProgram();
    }
    else if (canRun)
    {
        // set log file name
        if (m_program->ShouldLog())
        {
            m_logManager->SetCurrentLogName(m_program->GetInternalName());
        }

        m_logManager->OnClearLog();
        m_logManager->PrintLog(m_program->GetInternalName(), "Program started");
        StartProgram();
    }

    m_btnStart->setEnabled(canRun);
}

void ProgramManager::OnProgramFinished(int result)
{
    if (m_program && m_program->IsRunning())
    {
        if (result < 0)
        {
            m_logManager->PrintLog(m_program->GetInternalName(), "Program finished with an error", LOG_Error);
        }
        else
        {
            m_logManager->PrintLog(m_program->GetInternalName(), "Program finished successfully!", LOG_Success);
        }

        m_profileManager->PlaySound(m_startTime.secsTo(QDateTime::currentDateTime()) / 60);
        StopProgram();
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

void ProgramManager::OnManualOpen()
{
    if (!m_program) return;
    QDesktopServices::openUrl(QUrl::fromLocalFile(PROGRAM_MANUAL_PATH + m_program->GetInternalName() + ".pdf"));
}

void ProgramManager::OnUpTimeUpdate()
{
    qint64 const secs = m_startTime.secsTo(QDateTime::currentDateTime());
    QTime const time = QTime(0, 0).addSecs(secs);
    m_labelUpTime->setText(time.toString("hh:mm:ss"));
}

void ProgramManager::OnStatsEdit()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(PROGRAM_STATS_INI));
}

void ProgramManager::OnStatsReset()
{
    if (!m_program) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(this, "Reset Stats", "This will reset all stats for current program to 0, continue?\nIf you want to edit individual stats, press Edit.", QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        UpdateStats(true);
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
    settings.insert("CurrentProgram", m_programList->currentItem() ? m_programList->currentItem()->text() : "");

    JsonHelper::WriteSetting("ProgramSettings", settings);
}

void ProgramManager::StartProgram()
{
    if (!m_program || m_program->IsRunning() || !m_program->CanRun()) return;

    m_btnStart->setText("Stop Program (F5)");
    m_btnResetDefault->setEnabled(false);
    m_programCategory->setEnabled(false);
    m_programList->setEnabled(false);
    m_settingsParent->setEnabled(m_program->CanEditWhileRunning());
    m_startTime = QDateTime::currentDateTime();
    m_upTimer.start();
    OnUpTimeUpdate();
    m_program->Start();

    emit notifyStartStop();
}

void ProgramManager::StopProgram()
{
    m_logManager->SetCurrentLogName("");
    if (!m_program || !m_program->IsRunning()) return;

    m_program->Stop();
    m_upTimer.stop();
    m_btnStart->setText("Start Program (F5)");
    m_btnResetDefault->setEnabled(m_program->HaveSavedSettings());
    m_programCategory->setEnabled(true);
    m_programList->setEnabled(true);
    m_settingsParent->setEnabled(true);

    emit notifyStartStop();
}

template<class T>
void ProgramManager::RegisterProgram()
{
    QString const category = T::GetCategory();
    QString const name = T::GetName();

    QStringList& list = m_categoryToPrograms[category];
    list.push_back(name);

    m_programCtors[category + name] = []()->Program::ProgramBase* { return new T(); };
}

void ProgramManager::RemoveProgram()
{
    m_labelUpTime->setText("00:00:00");
    ClearStats();

    if (!m_program) return;

    if (m_program->IsRunning())
    {
        StopProgram();
    }

    m_program->SaveSettings();

    // remove settings layout
    while (m_settingsLayout->count())
    {
        delete m_settingsLayout->takeAt(0)->widget();
    }

    delete m_program;
    m_program = Q_NULLPTR;

    m_btnResetDefault->setEnabled(false);
}
