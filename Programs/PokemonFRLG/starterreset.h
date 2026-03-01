#ifndef FRLG_STARTERRESET_H
#define FRLG_STARTERRESET_H

#include "../programbase.h"
#include "Settings/settingspinbox.h"

namespace Program::PokemonFRLG
{
class StarterReset : public ProgramBase
{
public:
    explicit StarterReset(QObject *parent = nullptr);

    static QString GetCategory() { return "Pokemon Fire Red/Leaf Green"; }
    static QString GetName() { return "Starter Reset"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    void RegisterStats() override;
    QString GetInternalName() const override { return "FRLG-StarterReset"; }
    QString GetDescription() const override {
        return "Soft reset starter Pokemon until a shiny is found";
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

        GetStarter,
        ConfirmStarter,
        MenuToStarter,
        CheckStarter,

        Capture,
    };

private slots:
    void OnCommandFinished() override;
    void OnFrameCaptureMatched(bool matched) override;

private: // function
    void StateSoftReset();
    void GetNextPermutation();

private: // members
    Setting::SettingSpinBox* m_seedFrame = Q_NULLPTR;
    Setting::SettingSpinBox* m_advanceFrame = Q_NULLPTR;
    Setting::SettingSpinBox* m_confirmDelay = Q_NULLPTR;
    int m_currentMaxFrame = 0;

    State m_state;

    Stat m_statReset;
    Stat m_statShiny;
};
}

#endif // FRLG_STARTERRESET_H
