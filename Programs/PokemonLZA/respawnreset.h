#ifndef PLZA_RESPAWNRESET_H
#define PLZA_RESPAWNRESET_H

#include "../programbase.h"
#include "Settings/settingdoublespinbox.h"
#include "Types/categorytype.h"

namespace Program::PokemonLZA
{
class RespawnReset : public ProgramBase
{
public:
    explicit RespawnReset(QObject *parent = nullptr);

    static CategoryType GetCategory() { return CT_PLZA; }
    static QString GetName() { return "Respawn Reset"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    void RegisterStats() override;
    QString GetInternalName() const override { return "PLZA-RespawnReset"; }
    QString GetDescription() const override {
        return "Restart game until a Pokemon respawns as shiny";
    }

    bool RequireSerial() const override { return true; }
    bool RequireVideo() const override { return true; }
    bool RequireAudio() const override { return true; }

    void Start() override;
    void Stop() override;

private: // types
    enum class State
    {
        Restart,
        TitleScreen,
        GameLoadStart,
        GameLoadWait,

        Detect,
        Capture,
    };

private slots:
    void OnCommandFinished(Module::Common::RunCommand* module) override;
    void OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched) override;
    void OnWaitTimeout() override;
    void OnSoundDetected(int id) override;

private: // functions

private: // members
    Setting::SettingDoubleSpinBox* m_waitTime = Q_NULLPTR;

    State m_state;
    int m_shinySoundID = 0;

    Stat m_statReset;
    Stat m_statShiny;
};
}

#endif // PLZA_RESPAWNRESET_H
