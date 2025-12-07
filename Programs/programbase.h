#ifndef PROGRAMBASE_H
#define PROGRAMBASE_H

#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QObject>

#include "Managers/logmanager.h"
#include "Managers/serialmanager.h"
#include "Managers/vlcmanager.h"
#include "Programs/Settings/settingcombobox.h"
#include "Programs/Settings/settinglineedit.h"
#include "Programs/Settings/settingtextedit.h"

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
    bool HasSettings() { return !m_settings.empty(); }

signals:
    void notifyCanRun(bool);
    void notifyFinished();

protected slots:
    void OnCanRunChanged();

protected:
    void PrintLog(QString const& log);

    void AddSingleItem(QBoxLayout* layout, QWidget* widget);
    QLabel* AddSingleText(QBoxLayout* layout, QString const& str, bool isBold);
    void AddSingleSetting
    (
        QBoxLayout *layout,
        QString const& name,
        QString const& description,
        QWidget* setting,
        bool isHorizontal
    );

    SettingComboBox* AddComboBox
    (
        QBoxLayout *layout,
        QString const& name,
        QString const& description,
        QString const& settingName,
        QStringList const& list
    );

    SettingLineEdit* AddLineEdit
    (
        QBoxLayout *layout,
        QString const& name,
        QString const& description,
        QString const& settingName
    );

    SettingTextEdit* AddTextEdit
    (
        QBoxLayout *layout,
        QString const& name,
        QString const& description,
        QString const& settingName
    );

protected:
    LogManager*         m_logManager = Q_NULLPTR;
    SerialManager*      m_serialManager = Q_NULLPTR;
    AudioManager*       m_audioManager = Q_NULLPTR;
    VlcManager*         m_vlcManager = Q_NULLPTR;

    bool m_started = false;

private:
    QList<SettingBase*> m_settings;
};

#endif // PROGRAMBASE_H
