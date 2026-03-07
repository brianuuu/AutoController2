#ifndef FRLG_GIFTRESET_H
#define FRLG_GIFTRESET_H

#include "permutationbase.h"
#include "Settings/settingcheckbox.h"

namespace Program::PokemonFRLG
{
class GiftReset : public PermutationBase
{
public:
    explicit GiftReset(QObject *parent = nullptr);

    static QString GetCategory() { return "Pokemon Fire Red/Leaf Green"; }
    static QString GetName() { return "Gift Reset"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    QString GetInternalName() const override { return "FRLG-GiftReset"; }
    QString GetDescription() const override {
        return "Soft reset gift Pokemon until a shiny is found";
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
    void OnCommandFinished() override;
    void OnFrameCaptureMatched(bool matched) override;

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
