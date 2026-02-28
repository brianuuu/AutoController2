#ifndef FRLG_STARTERRESET_H
#define FRLG_STARTERRESET_H

#include "../programbase.h"

namespace Program::PokemonFRLG
{
class StarterReset : public ProgramBase
{
public:
    explicit StarterReset(QObject *parent = nullptr);

    static QString GetCategory() { return "Pokemon Fire Red/Leaf Green"; }
    static QString GetName() { return "Starter Reset"; }

    // from ProgramBase
    void RegisterStats() override;
    QString GetInternalName() const override { return "FRLG-StarterReset"; }
    QString GetDescription() const override {
        return "Soft reset starter Pokemon until a shiny is found";
    }

    bool RequireSerial() const override { return true; }
    bool RequireVideo() const override { return true; }
    bool RequireAudio() const override { return false; }

    bool BypassBorderCheck() const override { return true; }

    void Start() override;
    void Stop() override;

private: // types
    enum class State
    {
        SoftReset,
        Save,

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

private: // members
    State m_state;

    int m_statReset = 0;
    int m_statShiny = 0;
};
}

#endif // FRLG_STARTERRESET_H
