#ifndef CUSTOMCOMMAND_H
#define CUSTOMCOMMAND_H

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

private:
    QComboBox* m_list = Q_NULLPTR;
};
}

#endif // CUSTOMCOMMAND_H
