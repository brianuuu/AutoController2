#ifndef FRLG_PRIZECORNERRESET_H
#define FRLG_PRIZECORNERRESET_H

#include "Types/categorytype.h"
#include "permutationbase.h"
#include "Settings/settingspinbox.h"

namespace Program::PokemonFRLG
{
class PrizeCornerReset : public PermutationBase
{
public:
    explicit PrizeCornerReset(QObject *parent = nullptr);

    static CategoryType GetCategory() { return CT_FRLG; }
    static QString GetName() { return "Prize Corner Reset"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    void RegisterStats() override;
    QString GetInternalName() const override { return "FRLG-PrizeCornerReset"; }
    QString GetDescription() const override {
        return "Soft reset Prize Corner Pokemon until a shiny is found";
    }

    void Start() override;
    void Stop() override;

private: // types
    enum class State
    {
        SoftReset,
        TitleScreen,

        YesNoBox,
        WaitDialogue,

        PokemonSummary,
        CheckShiny,

        Capture,
    };

private slots:
    void OnCommandFinished(Module::Common::RunCommand* module) override;
    void OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched) override;
    void OnWaitTimeout() override;

private: // function
    void StateSoftReset();
    void StateWaitDialogue();
    void StateGetPrize();

private: // members
    Setting::SettingSpinBox* m_slot = Q_NULLPTR;
    Setting::SettingSpinBox* m_count = Q_NULLPTR;
    int m_currentCount = 0;
    int m_dialogCount = 0;

    State m_state;
    Stat m_statPrize;
};
}

#endif // FRLG_PRIZECORNERRESET_H
