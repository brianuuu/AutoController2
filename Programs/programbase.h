#ifndef PROGRAMBASE_H
#define PROGRAMBASE_H

#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QObject>

#include "Managers/managercollection.h"
#include "Programs/Settings/settingbase.h"

class ProgramBase : public QObject
{
    Q_OBJECT
public:
    explicit ProgramBase(QObject *parent = nullptr);

    void LoadSettings();
    void SaveSettings() const;

    virtual void PopulateSettings(QBoxLayout* layout) = 0;
    virtual QString GetInternalName() const = 0;
    virtual QString GetDescription() const = 0;

    virtual bool RequireSerial() const = 0;
    virtual bool RequireVideo() const = 0;
    virtual bool RequireAudio() const = 0;

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
    void notifyFinished();

protected slots:
    void OnCanRunChanged();

protected:
    void PrintLog(QString const& log);

    void AddItem(QBoxLayout* layout, QWidget* widget);
    QLabel* AddText(QBoxLayout* layout, QString const& str, bool isBold);
    void AddSetting(QBoxLayout *layout, QString const& name, QString const& description, QWidget* setting, bool isHorizontal);
    void AddSettings(QBoxLayout *layout, QString const& name, QString const& description, QList<QWidget*> settings, bool isHorizontal);

protected:
    LogManager*         m_logManager = Q_NULLPTR;
    SerialManager*      m_serialManager = Q_NULLPTR;
    AudioManager*       m_audioManager = Q_NULLPTR;
    VlcManager*         m_vlcManager = Q_NULLPTR;

    bool m_started = false;
    QList<SettingBase*> m_savedSettings;
};

#endif // PROGRAMBASE_H
