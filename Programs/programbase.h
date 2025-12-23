#ifndef PROGRAMBASE_H
#define PROGRAMBASE_H

#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QObject>

#include "Enums/system.h"
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

    virtual bool CanControlWhileRunning() { return false; }
    virtual bool CanEditWhileRunning() { return false; }
    virtual bool CanRun() const;

    virtual void ResetDefault();

    virtual void Start();
    virtual void Stop();

    bool IsRunning() { return m_started; }
    bool HaveSavedSettings() { return !m_savedSettings.empty(); }

    bool ValidSerial() const;
    bool ValidVideo() const;
    bool ValidAudio() const;

signals:
    void notifyCanRun(bool);
    void notifyStarted();
    void notifyFinished(int);
    void notifyLog(QString const& category, QString const& log, LogType type = LOG_Normal) const;

protected slots:
    void OnCanRunChanged();

protected:
    void PrintLog(QString const& log, LogType type = LOG_Normal) const;

    template<typename T, typename Func, typename... Args>
    T* AddModule(Func func, Args... args)
    {
        T* module = new T(args...);
        connect(module, &QThread::finished, this, func);
        m_modules.insert(module);
        module->moveToThread(module);
        module->start();
        return module;
    }
    void ClearModule(Module::ModuleBase*& module);
    void ClearModules();

    QLabel* AddText(QBoxLayout* layout, QString const& str, bool isBold);
    void AddSetting(QBoxLayout *layout, QString const& name, QString const& description, QWidget* setting, bool isHorizontal);
    void AddSettings(QBoxLayout *layout, QString const& name, QString const& description, QList<QWidget*> settings, bool isHorizontal);
    void AddSpacer(QBoxLayout *layout);


protected:
    SerialManager*      m_serialManager = Q_NULLPTR;
    AudioManager*       m_audioManager = Q_NULLPTR;
    VlcManager*         m_vlcManager = Q_NULLPTR;

    bool m_started = false;
    QSet<Setting::SettingBase*> m_savedSettings;
    QSet<Module::ModuleBase*> m_modules;
};
}

#endif // PROGRAMBASE_H
