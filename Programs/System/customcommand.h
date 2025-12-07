#ifndef CUSTOMCOMMAND_H
#define CUSTOMCOMMAND_H

#include <QRegularExpressionValidator>

#include "../programbase.h"

namespace System
{
class CustomCommand : public ProgramBase
{
    Q_OBJECT
public:
    explicit CustomCommand(QObject* parent = nullptr) : ProgramBase(parent) {}

    static QString GetCategory() { return "System"; }
    static QString GetName() { return "Custom Command"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    QString GetInternalName() const override { return "System-CustomCommand"; }
    QString GetDescription() const override {
        return "Run a pre-made command, or make custom commands.";
    }

    bool RequireSerial() const override { return true; }
    bool RequireVideo() const override { return false; }
    bool RequireAudio() const override { return false; }

private:
    QComboBox* m_list = Q_NULLPTR;
    QLineEdit* m_command = Q_NULLPTR;
};
}

#endif // CUSTOMCOMMAND_H
