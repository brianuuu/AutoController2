#ifndef DEVCOMMAND_H
#define DEVCOMMAND_H

#include <QFileDialog>
#include <QPushButton>
#include <QRegularExpressionValidator>

#include "../programbase.h"
#include "Settings/System/settingcommand.h"
#include "Settings/System/settingpreset.h"
#include "Settings/settingcombobox.h"
#include "Settings/settinglineedit.h"
#include "Types/categorytype.h"

namespace Program::Development
{
class DevCommand : public ProgramBase
{
    Q_OBJECT
public:
    explicit DevCommand(QObject* parent = nullptr);

    static CategoryType GetCategory() { return CT_Development; }
    static QString GetName() { return "Command Maker"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    QString GetInternalName() const override { return "Dev-CommandMaker"; }
    QString GetDescription() const override {
        return "Test and create command for RunCommand modules";
    }

    bool RequireSerial() const override { return true; }
    bool RequireVideo() const override { return false; }
    bool RequireAudio() const override { return false; }

    bool CanRun() const override;

    void Start() override;
    void Stop() override;

private slots:
    void OnListChanged(QString const& str);
    void OnCommandChanged();
    void OnCommandSave();

private:
    void VerifyCommand();

private:
    Setting::System::SettingPreset* m_list = Q_NULLPTR;
    QList<Setting::System::SettingCommand*> m_commandSettings;

    QPushButton* m_btnSave = Q_NULLPTR;
    QPushButton* m_btnDelete = Q_NULLPTR;
    QPushButton* m_btnDirectory = Q_NULLPTR;

    bool m_validCommand = false;
};

}

#endif // DEVCOMMAND_H
