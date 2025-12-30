#ifndef PROGRAMBASE_H
#define PROGRAMBASE_H

#include <QComboBox>
#include <QElapsedTimer>
#include <QLabel>
#include <QLayout>
#include <QMetaEnum>
#include <QObject>
#include <QTimer>

#include "Types/logtype.h"
#include "Managers/managercollection.h"
#include "Programs/Modules/modulebase.h"
#include "Programs/Settings/settingbase.h"

namespace Program
{
class ProgramBase : public QObject
{
    Q_OBJECT
public:
    explicit ProgramBase(QObject *parent = nullptr);
    ~ProgramBase();

    void LoadSettings();
    void SaveSettings() const;

    virtual void PopulateSettings(QBoxLayout* layout) = 0;
    virtual QString GetInternalName() const = 0;
    virtual QString GetDescription() const = 0;

    virtual bool RequireSerial() const = 0;
    virtual bool RequireVideo() const = 0;
    virtual bool RequireAudio() const = 0;

    virtual bool CanControlWhileRunning() const { return false; }
    virtual bool CanEditWhileRunning() const { return false; }
    virtual bool CanRun() const;

    virtual void ResetDefault();

    virtual void Start();
    virtual void Stop();

    bool IsRunning() const { return m_started; }
    bool HaveSavedSettings() const { return !m_savedSettings.empty(); }

    bool ValidSerial() const;
    bool ValidVideo() const;
    bool ValidAudio() const;

signals:
    void notifyCanRun(bool);
    void notifyStarted();
    void notifyFinished(int);
    void notifyLog(QString const& category, QString const& log, LogType type = LOG_Normal) const;

public slots:
    void OnCanRunChanged();
    void OnModuleFinishQuit();
    bool OnModuleErrorQuit();

protected slots:
    virtual void OnWaitTimeout() {}

protected:
    void PrintLog(QString const& log, LogType type = LOG_Normal) const;

    template<typename T, typename Func, typename... Args>
    T* AddModule(Func func, Args... args)
    {
        T* module = new T(args...);
        connect(module, &QThread::finished, this, func);
        AddModule(module);
        return module;
    }
    void AddModule(Module::ModuleBase* module);
    void ClearModule(QObject* sender);
    void ClearModule(Module::ModuleBase* module);
    void ClearModules();

    template<typename T>
    T SetState(T state, QString const& log = "")
    {
        QString fullLog = QString("State = ") + QMetaEnum::fromType<T>().valueToKey(state);
        if (!log.isEmpty())
        {
            fullLog += ": " + log;
        }
        PrintLog(fullLog, LOG_State);
        return state;
    }

public:
    static QLabel* AddText(QBoxLayout* layout, QString const& str, bool isBold);
    static void AddSetting(QBoxLayout *layout, QString const& name, QString const& description, QWidget* setting, bool isHorizontal);
    static void AddSettings(QBoxLayout *layout, QString const& name, QString const& description, QList<QWidget*> settings, bool isHorizontal);
    static void AddSeparator(QBoxLayout *layout);
    static void AddSpacer(QBoxLayout *layout);
    static void CleanOCRFiles();

protected:
    ProfileManager*     m_profileManager = Q_NULLPTR;
    SerialManager*      m_serialManager = Q_NULLPTR;
    AudioManager*       m_audioManager = Q_NULLPTR;
    VlcManager*         m_vlcManager = Q_NULLPTR;

    bool m_started = false;
    QTimer m_timer;
    QElapsedTimer m_elapsedTimer;
    QSet<Setting::SettingBase*> m_savedSettings;
    QSet<Module::ModuleBase*> m_modules;
};
}

#endif // PROGRAMBASE_H
