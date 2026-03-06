#ifndef FRLG_GIFTRESET_H
#define FRLG_GIFTRESET_H

#include "../programbase.h"
#include "Settings/settingcheckbox.h"
#include "Settings/settingspinbox.h"

namespace Program::PokemonFRLG
{
class GiftReset : public ProgramBase
{
public:
    explicit GiftReset(QObject *parent = nullptr);

    static QString GetCategory() { return "Pokemon Fire Red/Leaf Green"; }
    static QString GetName() { return "Gift Reset"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    void RegisterStats() override;
    QString GetInternalName() const override { return "FRLG-GiftReset"; }
    QString GetDescription() const override {
        return "Soft reset gift Pokemon until a shiny is found";
    }

    bool RequireSerial() const override { return true; }
    bool RequireVideo() const override { return true; }
    bool RequireAudio() const override { return false; }

    bool ShouldLog() const override { return true; }
    bool BypassBorderCheck() const override { return true; }

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
    void GetNextPermutation();

private: // members
    Setting::SettingSpinBox* m_seedFrame = Q_NULLPTR;
    Setting::SettingSpinBox* m_advanceFrame = Q_NULLPTR;
    Setting::SettingCheckBox* m_accept = Q_NULLPTR;
    int m_currentMaxFrame = 0;
    int m_dialogCount = 0;

    State m_state;

    Stat m_statReset;
    Stat m_statShiny;
};
}

#endif // FRLG_GIFTRESET_H
