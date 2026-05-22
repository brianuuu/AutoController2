#include "programmanager.h"

#include "../ui_mainwindow.h"
#include "Helpers/jsonhelper.h"
#include "Managers/discordmanager.h"
#include "Managers/keyboardmanager.h"
#include "Managers/logmanager.h"
#include "Managers/profilemanager.h"
#include "Managers/videomanager.h"

#include "Programs/Development/devcommand.h"
#include "Programs/Development/devframecapture.h"
#include "Programs/Development/devsounddetection.h"
#include "Programs/MMSFLC/ciphercollector.h"
#include "Programs/MMSFLC/itemcardscollector.h"
#include "Programs/PokemonFRLG/giftreset.h"
#include "Programs/PokemonFRLG/nuggetfarmer.h"
#include "Programs/PokemonFRLG/overworldshiny.h"
#include "Programs/PokemonFRLG/pickupfarmer.h"
#include "Programs/PokemonFRLG/prizecornerreset.h"
#include "Programs/PokemonFRLG/rngmanipulation.h"
#include "Programs/PokemonFRLG/starterreset.h"
#include "Programs/PokemonFRLG/staticreset.h"
#include "Programs/PokemonLZA/donutmaker.h"
#include "Programs/PokemonLZA/respawnreset.h"
#include "Programs/System/camerachecker.h"
#include "Programs/System/commandrecorder.h"
#include "Programs/System/customcommand.h"

#define PROGRAM_STATS_JSON "../Stats.json"
#define STREAM_COUNTER_PATH "../StreamCounters/"

void ProgramManager::Initialize(Ui::MainWindow *ui)
{
    m_logManager = ManagerCollection::GetManager<LogManager>();
    m_discordManager = ManagerCollection::GetManager<DiscordManager>();
    m_profileManager = ManagerCollection::GetManager<ProfileManager>();
    m_videoManager = ManagerCollection::GetManager<VideoManager>();

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
    m_upTimer.setTimerType(Qt::PreciseTimer);

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
    RegisterProgram<Program::Development::DevSoundDetection>();
    RegisterProgram<Program::MMSFLC::CipherCollector>();
    RegisterProgram<Program::MMSFLC::ItemCardsCollector>();
    RegisterProgram<Program::PokemonFRLG::GiftReset>();
    RegisterProgram<Program::PokemonFRLG::NuggetFarmer>();
    RegisterProgram<Program::PokemonFRLG::OverworldShiny>();
    RegisterProgram<Program::PokemonFRLG::PickupFarmer>();
    RegisterProgram<Program::PokemonFRLG::PrizeCornerReset>();
    RegisterProgram<Program::PokemonFRLG::RNGManipulation>();
    RegisterProgram<Program::PokemonFRLG::StarterReset>();
    RegisterProgram<Program::PokemonFRLG::StaticReset>();
    RegisterProgram<Program::PokemonLZA::DonutMaker>();
    RegisterProgram<Program::PokemonLZA::RespawnReset>();
    RegisterProgram<Program::System::CameraChecker>();
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

    MigrateStatsToJson();
    LoadSettings();
}

bool ProgramManager::OnCloseEvent()
{
    SaveSettings();
    RemoveProgram();
    return true;
}

void ProgramManager::RegisterStat(Stat &stat)
{
    if (!m_program) return;

    m_stats.push_back(&stat);
    ReadStat(stat);

    connect(&stat, &Stat::notifyStatChanged, this, [this]{ UpdateStats(); SaveStats(); });
    UpdateStats();
}

void ProgramManager::ReadStat(Stat &stat)
{
    QJsonObject json = JsonHelper::ReadJson(PROGRAM_STATS_JSON);
    QJsonObject stats = JsonHelper::ReadObject(json, m_program->GetInternalName());

    QVariant value;
    if (JsonHelper::ReadValue(stats, stat.GetName(), value))
    {
        // avoid emit signal so it doesn't trigger save
        stat.SetTotal(value.toInt(), false);
    }
}

void ProgramManager::UpdateStats() const
{
    if (!m_program || m_stats.isEmpty())
    {
        m_labelStats->setText("N/A");
        m_btnStatsReset->setEnabled(false);
        return;
    }

    // Update label
    QString statsStr;
    m_btnStatsEdit->setEnabled(!m_program->IsRunning());
    m_btnStatsReset->setEnabled(true);
    int i = 0;
    for (Stat const* stat : std::as_const(m_stats))
    {
        if (stat->GetTotal() == 0 && stat->GetHideZero())
        {
            continue;
        }

        QString const& key = stat->GetName();
        if (i != 0)
        {
            statsStr += ", ";
        }

        statsStr += key + ": " + stat->GetString() + " (" + stat->GetTotalString() + ")";

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
                stream << stat->GetTotal();
                file.close();
            }
        }

        i++;
    }
    m_labelStats->setText(statsStr);
}

void ProgramManager::SaveStats()
{
    if (!m_program || m_stats.empty()) return;

    QJsonObject json = JsonHelper::ReadJson(PROGRAM_STATS_JSON);

    // save stats of the program
    QJsonObject stats;
    for (Stat const* stat : std::as_const(m_stats))
    {
        stats.insert(stat->GetName(), stat->GetTotal());
    }

    json.insert(m_program->GetInternalName(), stats);
    JsonHelper::WriteJson(PROGRAM_STATS_JSON, json);
}

void ProgramManager::ClearStats()
{
    // Delete all stream counter text files
    QDirIterator it(QString(STREAM_COUNTER_PATH));
    while (it.hasNext())
    {
        QString dir = it.next();
        QFile::remove(dir);
    }

    m_stats.clear();
    UpdateStats();
}

void ProgramManager::MigrateStatsToJson()
{
    QString const statsIni = "../Stats.ini";
    if (!QFile::exists(statsIni)) return;

    QJsonObject json = JsonHelper::ReadJson(PROGRAM_STATS_JSON);

    QSettings statsSettings(statsIni, QSettings::IniFormat, this);
    for (QString const& program : statsSettings.childGroups())
    {
        QJsonObject programStats;

        statsSettings.beginGroup(program);
        for (QString const& key : statsSettings.allKeys())
        {
            programStats.insert(key, statsSettings.value(key).toInt());
        }
        statsSettings.endGroup();

        json.insert(program, programStats);
    }

    JsonHelper::WriteJson(PROGRAM_STATS_JSON, json);
    QFile::remove(statsIni);
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

    if (category == CategoryToString(CT_FRLG))
    {
        if (!HasProgramRun("System-CameraChecker"))
        {
            QString message = "To run programs for " + category + ",\nplease run 'System/Camera Checker' first to make sure everything is working correctly.";
            QMessageBox::warning(this, "Warning", message, QMessageBox::Ok);
        }
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

    m_btnManual->setEnabled(QFile::exists(MANUAL_PATH + m_program->GetInternalName() + ".pdf"));
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

    auto fnSetPalette = [](QLabel* label, bool required, bool valid)
    {
        QPalette palette = label->palette();
        palette.setColor(QPalette::WindowText, required ? (valid ? LogTypeToColor(LOG_Success) : LogTypeToColor(LOG_Error)) : QGuiApplication::palette().windowText().color());
        label->setPalette(palette);
    };

    // highlight required elements
    fnSetPalette(m_labelSerial, m_program->RequireSerial(), m_program->ValidSerial());
    fnSetPalette(m_labelCamera, m_program->RequireVideo(), m_program->ValidVideo());
    fnSetPalette(m_labelAudio, m_program->RequireAudio(), m_program->ValidAudio());

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
        if (m_program->RequireVideo() && !m_program->BypassBorderCheck())
        {
            QImage const frame = m_videoManager->GetFrameData();

            bool xBorder = true;
            for (int x = 0; x < frame.width(); x++)
            {
                if ((frame.pixel(0,x) & 0x00FFFFFF) != 0 || (frame.pixel(frame.height() - 1,x) & 0x00FFFFFF) != 0) // top/bottom not black
                {
                    xBorder = false;
                    break;
                }
            }

            bool yBorder = true;
            for (int y = 0; y < frame.height(); y++)
            {
                if ((frame.pixel(0,y) & 0x00FFFFFF) != 0 || (frame.pixel(frame.width() - 1,y) & 0x00FFFFFF) != 0) // left/right not black
                {
                    yBorder = false;
                    break;
                }
            }

            if (xBorder || yBorder)
            {
                QMessageBox::critical(this, "Error", "Black border detected, please go to Switch Settings->TV Settings->Adjust Screen Size and set to 100%.");
                return;
            }
        }

        // set log file name
        if (m_program->ShouldLog())
        {
            m_logManager->SetCurrentLogName(m_program->GetInternalName());
        }

        m_logManager->OnShow();
        m_logManager->OnClearLog();
        m_logManager->PrintLog(m_program->GetInternalName(), "Program started");
        StartProgram();
    }

    m_btnStart->setEnabled(canRun);
}

void ProgramManager::OnProgramFinished(bool success, QString msg)
{
    if (m_program && m_program->IsRunning())
    {
        int const mins = GetUpTime() / 60;
        bool const sendDiscordMessage = mins >= m_discordManager->m_settingFinishSuppress->value();
        QString const upTime = " (Up time = " + m_labelUpTime->text() + ")";
        if (success)
        {
            m_program->SetHasRun();
            m_logManager->PrintLog(m_program->GetInternalName(), "Program finished successfully!" + upTime, LOG_Success);
            if (sendDiscordMessage)
            {
                m_program->SendDiscordMessage("Program Finished", false, true, false, LOG_Success);
            }
        }
        else
        {
            m_logManager->PrintLog(m_program->GetInternalName(), "Program finished with an error" + (msg.isEmpty() ? "" : (": " + msg)) + upTime, LOG_Error);
            if (sendDiscordMessage)
            {
                Discord::EmbedField embedField("Error Message", msg, false);
                m_program->SendDiscordMessage("Error Occured", false, true, true, LOG_Error, {embedField});
            }
        }

        m_profileManager->PlaySound(mins);
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
    QDesktopServices::openUrl(QUrl::fromLocalFile(MANUAL_PATH + m_program->GetInternalName() + ".pdf"));
}

void ProgramManager::OnUpTimeUpdate()
{
    qint64 const secs = GetUpTime();
    qint64 const mins = secs / 60;
    qint64 const hours = mins / 60;

    QTime const time = QTime(0, 0).addSecs(secs);
    m_labelUpTime->setText(time.toString("hh:mm:ss"));

    // long running discord message
    if (m_program && hours > m_upHour && m_discordManager->m_settingHourlyUpdate->isChecked())
    {
        m_upHour = hours;
        m_program->SendDiscordMessage("Program Status", false, true, false, LOG_Normal);
    }
}

void ProgramManager::OnStatsEdit()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(PROGRAM_STATS_JSON));
}

void ProgramManager::OnStatsReset()
{
    if (!m_program) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(this, "Reset Stats", "This will reset all stats for current program to 0, continue?\nIf you want to edit individual stats, press Edit.", QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        for (Stat* const stat : std::as_const(m_stats))
        {
            // this sends UpdateStats() signal
            stat->Reset();
        }
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

    // make sure stat matches json and reset value for current run
    for (Stat* stat : std::as_const(m_stats))
    {
        ReadStat(*stat);
        stat->SetValue(0, false);
    }
    UpdateStats();

    m_btnStart->setText("Stop Program (F5)");
    m_btnResetDefault->setEnabled(false);
    m_btnStatsEdit->setEnabled(false);
    m_programCategory->setEnabled(false);
    m_programList->setEnabled(false);
    m_settingsParent->setEnabled(m_program->CanEditWhileRunning());
    m_startTime = QDateTime::currentDateTime();
    m_upTimer.start();
    m_upHour = 0;
    OnUpTimeUpdate();
    m_program->Start();

    emit notifyStartStop();
}

void ProgramManager::StopProgram()
{
    SaveStats();
    m_logManager->SetCurrentLogName("");
    if (!m_program || !m_program->IsRunning()) return;

    m_program->Stop();
    m_upTimer.stop();
    m_btnStart->setText("Start Program (F5)");
    m_btnResetDefault->setEnabled(m_program->HaveSavedSettings());
    m_btnStatsEdit->setEnabled(true);
    m_programCategory->setEnabled(true);
    m_programList->setEnabled(true);
    m_settingsParent->setEnabled(true);

    emit notifyStartStop();
}

template<class T>
void ProgramManager::RegisterProgram(bool beta)
{
#ifndef QT_DEBUG
    if (beta && !IS_BETA) return;
#endif

    QString const category = CategoryToString(T::GetCategory());
    QString const name = T::GetName() + (beta ? " (BETA)" : "");

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

bool ProgramManager::HasProgramRun(const QString &name) const
{
    QJsonObject allSettings = JsonHelper::ReadSetting("ProgramSettings");
    QJsonObject settings = JsonHelper::ReadObject(allSettings, name);

    QVariant hasRun;
    return JsonHelper::ReadValue(settings, "HasRun", hasRun) && hasRun.toBool();
}
