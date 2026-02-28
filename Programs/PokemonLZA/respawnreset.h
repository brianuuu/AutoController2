#ifndef PLZA_RESPAWNRESET_H
#define PLZA_RESPAWNRESET_H

#include "../programbase.h"
#include "Settings/settingdoublespinbox.h"

namespace Program::PokemonLZA
{
class RespawnReset : public ProgramBase
{
public:
    explicit RespawnReset(QObject *parent = nullptr);

    static QString GetCategory() { return "Pokemon Legends: Z-A"; }
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
    void OnCommandFinished();
    void OnFrameCaptureMatched(bool matched);
    void OnWaitTimeout() override;
    void OnSoundDetected(int id) override;

private: // functions

private: // members
    Setting::SettingDoubleSpinBox* m_waitTime = Q_NULLPTR;

    State m_state;
    int m_shinySoundID = 0;

    int m_statReset = 0;
    int m_statShiny = 0;
};
}

#endif // PLZA_RESPAWNRESET_H
