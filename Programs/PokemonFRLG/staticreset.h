#ifndef FRLG_STATICRESET_H
#define FRLG_STATICRESET_H

#include "Types/categorytype.h"
#include "permutationbase.h"
#include "Settings/settingcheckbox.h"

namespace Program::PokemonFRLG
{
class StaticReset : public PermutationBase
{
public:
    explicit StaticReset(QObject *parent = nullptr);

    static CategoryType GetCategory() { return CT_FRLG; }
    static QString GetName() { return "Static Reset"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    QString GetInternalName() const override { return "FRLG-StaticReset"; }
    QString GetDescription() const override {
        return "Soft reset static Pokemon until a shiny is found, including Legendary Pokemon, Snorlax etc.";
    }

    void Start() override;
    void Stop() override;

private: // types
    enum class State
    {
        SoftReset,
        TitleScreen,

        EncounterTrigger,
        EncounterStart,
        EncounterWait,
        Listen,

        Capture,
    };

private slots:
    void OnCommandFinished(Module::Common::RunCommand* module) override;
    void OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched) override;
    void OnSubModuleResult(Module::SubModuleBase* module, int result) override;
    void OnWaitTimeout() override;
    void OnSoundDetected(int id) override;

private: // function
    void StateSoftReset();

private: // members
    Setting::SettingCheckBox* m_moveUp = Q_NULLPTR;

    State m_state;
    int m_shinySoundID = 0;
    qint64 m_battleDelay = 0;
};
}

#endif // FRLG_STATICRESET_H
