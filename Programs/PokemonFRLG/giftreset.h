#ifndef FRLG_GIFTRESET_H
#define FRLG_GIFTRESET_H

#include "Types/categorytype.h"
#include "permutationbase.h"
#include "Settings/settingcheckbox.h"

namespace Program::PokemonFRLG
{
class GiftReset : public PermutationBase
{
public:
    explicit GiftReset(QObject *parent = nullptr);

    static CategoryType GetCategory() { return CT_FRLG; }
    static QString GetName() { return "Gift Reset"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    QString GetInternalName() const override { return "FRLG-GiftReset"; }
    QString GetDescription() const override {
        return "Soft reset gift Pokemon (including fossil) until a shiny is found";
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
        CheckPokemon,

        Capture,
    };

private slots:
    void OnCommandFinished(Module::Common::RunCommand* module) override;
    void OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched) override;

private: // function
    void StateSoftReset();
    void StateWaitDialogue();

private: // members
    Setting::SettingCheckBox* m_accept = Q_NULLPTR;
    int m_dialogCount = 0;

    State m_state;
};
}

#endif // FRLG_GIFTRESET_H
