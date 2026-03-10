#ifndef FRLG_NUGGETFARMER_H
#define FRLG_NUGGETFARMER_H

#include "../programbase.h"
#include "Settings/settingspinbox.h"
#include "Types/categorytype.h"

namespace Program::PokemonFRLG
{
class NuggetFarmer : public ProgramBase
{
public:
    explicit NuggetFarmer(QObject *parent = nullptr) : ProgramBase{parent} {}

    static CategoryType GetCategory() { return CT_FRLG; }
    static QString GetName() { return "Nugget Farmer"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    void RegisterStats() override;
    QString GetInternalName() const override { return "FRLG-NuggetFarmer"; }
    QString GetDescription() const override {
        return "Farm nuggets from Team Rocket Grunt on Nugget Bridge until the bag is full";
    }

    bool RequireSerial() const override { return true; }
    bool RequireVideo() const override { return true; }
    bool RequireAudio() const override { return false; }

    bool ShouldLog() const override { return true; }
    bool BypassBorderCheck() const override { return true; }

    void Start() override;
    void Stop() override;

private slots:
	void OnCommandFinished() override;
    void OnFrameCaptureMatched(bool matched) override;

private: // types
    enum class State
    {
        ToNuggetBridge,
        BattleBox,
        BattleLose,
        ReturnToPC,
    };

private: // function
    void StateToNuggetBridge();

private: // members
    Setting::SettingSpinBox* m_count;
    int m_currentCount = 0;

    Module::Common::RunCommand* m_moduleCommand = Q_NULLPTR;
    int m_dialogCount = 0;

	State m_state;
    Stat m_statNuggets;
};
}

#endif // FRLG_NUGGETFARMER_H
