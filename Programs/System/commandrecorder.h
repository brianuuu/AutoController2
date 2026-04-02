#ifndef SYSTEM_COMMANDRECORDER_H
#define SYSTEM_COMMANDRECORDER_H

#include "../programbase.h"
#include "Settings/settingcombobox.h"
#include "Settings/settingtextbrowser.h"
#include "Types/categorytype.h"

namespace Program::System
{
class CommandRecorder : public ProgramBase
{
public:
    explicit CommandRecorder(QObject *parent = nullptr);

    static CategoryType GetCategory() { return CT_System; }
    static QString GetName() { return "Command Recorder"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    QString GetInternalName() const override { return "System-CommandRecorder"; }
    QString GetDescription() const override {
        return "Records a command sequence from user input, this can then be used in Custom Command program.\n"
            "WARNING: This will not be accurate for long commands, especially for Gamepad.";
    }

    bool RequireSerial() const override { return true; }
    bool RequireVideo() const override { return false; }
    bool RequireAudio() const override { return false; }

    bool CanControlWhileRunning() const override { return true; }
    bool CanEditWhileRunning() const override { return true; }

    void Start() override;
    void Stop() override;

private slots:
    void OnUserInput(quint32 buttonFlag, QPointF lStick, QPointF rStick);
    void OnTextChanged();

private:
    void AppendCommand(QString const& command);

private:
    Setting::SettingComboBox* m_nothing = Q_NULLPTR;
    Setting::SettingTextBrowser* m_browser = Q_NULLPTR;
    QLabel* m_duration = Q_NULLPTR;

    quint32 m_buttonFlags = 0;
    QPointF m_lStick = QPointF();
    QPointF m_rStick = QPointF();
};
}

#endif // SYSTEM_COMMANDRECORDER_H
