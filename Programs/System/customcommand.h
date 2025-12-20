#ifndef CUSTOMCOMMAND_H
#define CUSTOMCOMMAND_H

#include <QDir>
#include <QFileDialog>
#include <QRegularExpressionValidator>
#include <QPushButton>

#include "../programbase.h"
#include "Programs/Settings/settingcombobox.h"
#include "Programs/Settings/settinglineedit.h"
#include "Programs/Settings/settingtextedit.h"

namespace System
{
class CustomCommand : public ProgramBase
{
    Q_OBJECT
public:
    explicit CustomCommand(QObject* parent = nullptr);

    static QString GetCategory() { return "System"; }
    static QString GetName() { return "Custom Command"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    QString GetInternalName() const override { return "System-CustomCommand"; }
    QString GetDescription() const override {
        return "Runs a pre-made command, or make custom commands.";
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
    void OnCommandEdited();
    void OnCommandFinished();
    void OnCommandSave();
    void OnCommandDelete();

private:
    void VerifyCommand();

private:
    SettingComboBox* m_list = Q_NULLPTR;
    SettingLineEdit* m_command = Q_NULLPTR;
    SettingTextEdit* m_description = Q_NULLPTR;
    QLabel* m_labelStatus = Q_NULLPTR;
    QPushButton* m_btnSave = Q_NULLPTR;
    QPushButton* m_btnDelete = Q_NULLPTR;

    bool m_validCommand = false;
};
}

#endif // CUSTOMCOMMAND_H
