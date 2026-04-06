#ifndef FRLG_PICKUPFARMER_H
#define FRLG_PICKUPFARMER_H

#include "../programbase.h"
#include "Settings/settingcheckbox.h"
#include "Settings/settingspinbox.h"
#include "Types/categorytype.h"

namespace Program::PokemonFRLG
{
class PickupFarmer : public ProgramBase
{
public:
    explicit PickupFarmer(QObject *parent = nullptr) : ProgramBase{parent} {}

    static CategoryType GetCategory() { return CT_FRLG; }
    static QString GetName() { return "Pickup Farmer"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    void RegisterStats() override;
    QString GetInternalName() const override { return "FRLG-PickupFarmer"; }
    QString GetDescription() const override {
        return "Indefinitely farm items using Pickup ability";
    }

    bool RequireSerial() const override { return true; }
    bool RequireVideo() const override { return true; }
    bool RequireAudio() const override { return false; }

    bool ShouldLog() const override { return true; }
    bool BypassBorderCheck() const override { return true; }

    void Start() override;
    void Stop() override;

private slots:
	void OnCommandFinished(Module::Common::RunCommand* module) override;
    void OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched) override;
    void OnSubModuleResult(Module::SubModuleBase* module, int result) override;
    void OnWaitTimeout() override;

private: // types
    enum class State
    {
        Heal,
        Move,
        EncounterStart,
        EncounterWait,
        BattleSelection,
        BattleFinish,
        FetchItem,
        Capture,
    };

private: // function
    void Restart();
    void StateHeal();
    void StateMove();
    void StateFetchItem();

private: // members
    Setting::SettingSpinBox* m_maxPP = Q_NULLPTR;
    Setting::SettingCheckBox* m_stopForShiny = Q_NULLPTR;

	State m_state;
    qint64 m_battleDelay = 0;
    int m_battleCount = 0;
    bool m_isUp = true;
    bool m_shouldRun = false;

    Stat m_statEncounter;
    Stat m_statShiny;
};
}

#endif // FRLG_PICKUPFARMER_H
