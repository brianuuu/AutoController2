#ifndef PROGRAMBASE_H
#define PROGRAMBASE_H

#include <QComboBox>
#include <QElapsedTimer>
#include <QLabel>
#include <QLayout>
#include <QMetaEnum>
#include <QObject>
#include <QTimer>

#include "External/QDiscord/Discord/Objects/Embed.h"
#include "Modules/moduleholder.h"
#include "Types/logtype.h"
#include "Types/stat.h"
#include "Managers/managercollection.h"
#include "Settings/settingbase.h"

namespace Program
{
class ProgramBase : public Module::ModuleHolder
{
    Q_OBJECT
public:
    explicit ProgramBase(QObject *parent = nullptr);
    ~ProgramBase();

    void LoadSettings();
    void SaveSettings() const;

    virtual void PopulateSettings(QBoxLayout* layout);
    virtual void RegisterStats() {}
    virtual QString GetInternalName() const = 0;
    virtual QString GetDescription() const = 0;

    virtual bool RequireSerial() const = 0;
    virtual bool RequireVideo() const = 0;
    virtual bool RequireAudio() const = 0;

    virtual bool ShouldLog() const { return false; }
    virtual bool BypassBorderCheck() const { return false; }
    virtual bool CanControlWhileRunning() const { return false; }
    virtual bool CanEditWhileRunning() const { return false; }
    virtual bool CanRun() const;

    virtual void ResetDefault();

    virtual void Start();
    virtual void Stop();

    bool IsRunning() const { return m_started; }
    bool HaveSavedSettings() const { return !m_savedSettings.empty(); }
    void SetHasRun() { m_hasRun = true; }

    bool ValidSerial() const;
    bool ValidVideo() const;
    bool ValidAudio() const;

    void SendDiscordMessage(QString const& title, bool isMention, bool dmOnly, bool hasImage, LogType type, QList<Discord::EmbedField> const& fields = {});

signals:
    void notifyCanRun(bool);
    void notifyStarted();
    void notifyFinished(bool success, QString msg = "");
    void notifyLog(QString const& category, QString const& log, LogType type = LOG_Normal) const;

public slots:
    void OnCanRunChanged();
    void OnModuleFinishQuit();
    bool OnModuleErrorQuit();

protected slots:
    virtual void OnCommandFinished() {}
    virtual void OnFrameCaptureMatched(bool matched) {}
    virtual void OnWaitTimeout() {}
    virtual void OnSoundDetected(int id) {}

protected:
    void RegisterStat(Stat& stat, QString const& name, bool hideZero = false);

    void PrintLog(QString const& log, LogType type = LOG_Normal) const;

    void UnhandedStateRunCommand();
    void UnhandedStateFrameCapture();

    template<typename T>
    T SetState(T state, QString const& log = "")
    {
        QString fullLog = "State = " + QString::number((int)state);
        if (!log.isEmpty())
        {
            fullLog += ": " + log;
        }
        PrintLog(fullLog, LOG_State);
        return state;
    }

    bool EnsureOCRDatabase(QString const& database);

public:
    static QLabel* AddText(QBoxLayout* layout, QString const& str, bool isBold, bool isBig = false);
    static QLabel* AddText(QBoxLayout* layout, QString const& str, bool isBold, QColor color, bool isBig = false);
    static void AddSetting(QBoxLayout *layout, QString const& name, QString const& description, QWidget* setting, bool isHorizontal = true);
    static void AddSettings(QBoxLayout *layout, QString const& name, QString const& description, QList<QWidget*> settings, bool isHorizontal = true);
    static void AddSeparator(QBoxLayout *layout);
    static void AddSpacer(QBoxLayout *layout);
    static void CleanOCRFiles();

protected:
    ProgramManager*     m_programManager = Q_NULLPTR;
    ProfileManager*     m_profileManager = Q_NULLPTR;
    DiscordManager*     m_discordManager = Q_NULLPTR;
    SerialManager*      m_serialManager = Q_NULLPTR;
    AudioManager*       m_audioManager = Q_NULLPTR;
    VideoManager*       m_videoManager = Q_NULLPTR;
    VlcManager*         m_vlcManager = Q_NULLPTR;

    bool m_started = false;
    bool m_hasRun = false; // only set if this program has successfully finished before
    QTimer m_timer;
    QElapsedTimer m_elapsedTimer;
    QSet<Setting::SettingBase*> m_savedSettings;
};
}

#endif // PROGRAMBASE_H
