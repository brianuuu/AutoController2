#ifndef TEMPLATE_H
#define TEMPLATE_H

#include "../programbase.h"

namespace Program::CATEGORY
{
class NAME_NO_SPACE : public ProgramBase
{
public:
    explicit NAME_NO_SPACE(QObject *parent = nullptr);

    static QString GetCategory() { return "CATEGORY"; }
    static QString GetName() { return "NAME_SPACE"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    QString GetInternalName() const override { return "CATEGORY-NAME_NO_SPACE"; }
    QString GetDescription() const override {
        return "DESCRIPTION";
    }

    bool RequireSerial() const override { return true; }
    bool RequireVideo() const override { return false; }
    bool RequireAudio() const override { return false; }

    void Start() override;
    void Stop() override;

private slots:


private: // function


private: // members

};
}

#endif // TEMPLATE_H
