#ifndef FRLG_OVERWORLDSHINY_H
#define FRLG_OVERWORLDSHINY_H

#include "../programbase.h"
#include "Settings/settingcombobox.h"
#include "Settings/settingspinbox.h"
#include "Types/categorytype.h"

namespace Program::PokemonFRLG
{
class OverworldShiny : public ProgramBase
{
public:
    explicit OverworldShiny(QObject *parent = nullptr) : ProgramBase{parent} {}

    static CategoryType GetCategory() { return CT_FRLG; }
    static QString GetName() { return "Overworld Shiny"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    void RegisterStats() override;
    QString GetInternalName() const override { return "FRLG-OverworldShiny"; }
    QString GetDescription() const override {
        return "Running back and forth in overworld for random encounters untiil a shiny is found";
    }

    bool RequireSerial() const override { return true; }
    bool RequireVideo() const override { return true; }
    bool RequireAudio() const override { return true; }

    bool ShouldLog() const override { return true; }
    bool BypassBorderCheck() const override { return true; }

    void Start() override;
    void Stop() override;

private slots:
    void OnTypeChanged(int index);
    void OnCommandFinished(Module::Common::RunCommand* module) override;
    void OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched) override;
    void OnSubModuleResult(Module::SubModuleBase* module, int result) override;
    void OnWaitTimeout() override;
    void OnSoundDetected(int id) override;

private: // types
    enum class State
    {
        Move,
        EncounterStart,
        EncounterWait,
        Listen,
        RunAway,
        Capture,
    };

    enum class Type
    {
        UpDown,
        LeftRight,
        SpinInPlace,
    };

private: // function
    void StateMove();

private: // members
    Setting::SettingComboBox* m_type = Q_NULLPTR;
    Setting::SettingSpinBox* m_moveTime = Q_NULLPTR;

    Module::Common::FrameCapture* m_moduleTop = Q_NULLPTR;
    Module::Common::FrameCapture* m_moduleBottom = Q_NULLPTR;
    bool m_blackTop = false;
    bool m_blackBottom = false;

    State m_state;
    int m_shinySoundID = 0;
    qint64 m_battleDelay = 0;
    bool m_isUp = true;

    Stat m_statEncounter;
    Stat m_statShiny;
};
}

#endif // FRLG_OVERWORLDSHINY_H
