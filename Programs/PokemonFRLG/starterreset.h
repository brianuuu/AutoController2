#ifndef FRLG_STARTERRESET_H
#define FRLG_STARTERRESET_H

#include "Types/categorytype.h"
#include "permutationbase.h"
#include "Settings/settingspinbox.h"

namespace Program::PokemonFRLG
{
class StarterReset : public PermutationBase
{
public:
    explicit StarterReset(QObject *parent = nullptr);

    static CategoryType GetCategory() { return CT_FRLG; }
    static QString GetName() { return "Starter Reset"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    QString GetInternalName() const override { return "FRLG-StarterReset"; }
    QString GetDescription() const override {
        return "Soft reset starter Pokemon until a shiny is found";
    }

    void Start() override;
    void Stop() override;

private: // types
    enum class State
    {
        SoftReset,
        TitleScreen,

        GetStarter,
        ConfirmStarter,
        MenuToStarter,
        CheckStarter,

        Capture,
    };

private slots:
    void OnCommandFinished(Module::Common::RunCommand* module) override;
    void OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched) override;

private: // function
    void StateSoftReset();

private: // members
    Setting::SettingSpinBox* m_confirmDelay = Q_NULLPTR;
    int m_dialogCount = 0;

    State m_state;
};
}

#endif // FRLG_STARTERRESET_H
