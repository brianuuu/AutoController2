#ifndef FRLG_RNGMANIPULATION_H
#define FRLG_RNGMANIPULATION_H

#include "../programbase.h"
#include "Settings/System/settingcommand.h"
#include "Settings/settingspinbox.h"
#include "Types/categorytype.h"

namespace Program::PokemonFRLG
{
class RNGManipulation : public ProgramBase
{
public:
    explicit RNGManipulation(QObject *parent = nullptr) : ProgramBase{parent} {}

    static CategoryType GetCategory() { return CT_FRLG; }
    static QString GetName() { return "RNG Manipulation"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    QString GetInternalName() const override { return "FRLG-RNGManipulation"; }
    QString GetDescription() const override {
        return "A helper program that presses A at precise times to do RNG manipulation";
    }

    bool RequireSerial() const override { return true; }
    bool RequireVideo() const override { return false; }
    bool RequireAudio() const override { return false; }

    bool CanRun() const override;

    void Start() override;
    void Stop() override;

private slots:
	void OnCommandFinished() override;

private: // types
    enum class State
    {
        TitleScreen,
        ContinueScreen,
        Flashback,
        CommandFlashback,
        OverworldWait,
        CommandComplete,
    };

private: // function


private: // members
    Setting::SettingSpinBox* m_tid = Q_NULLPTR;
    Setting::SettingSpinBox* m_sid = Q_NULLPTR;
    Setting::SettingSpinBox* m_seedTime = Q_NULLPTR;
    Setting::SettingSpinBox* m_seedCalibrate = Q_NULLPTR;
    Setting::SettingSpinBox* m_continueFrames = Q_NULLPTR;
    Setting::SettingSpinBox* m_continueCalibrate = Q_NULLPTR;
    Setting::SettingSpinBox* m_overworldFrames = Q_NULLPTR;
    Setting::System::SettingCommand* m_commandFlashback = Q_NULLPTR;
    Setting::System::SettingCommand* m_commandComplete = Q_NULLPTR;

    State m_state;
};
}

#endif // FRLG_RNGMANIPULATION_H
