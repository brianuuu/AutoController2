#ifndef FRLG_RNGMANIPULATION_H
#define FRLG_RNGMANIPULATION_H

#include <QPushButton>

#include "../programbase.h"
#include "Settings/System/settingcommand.h"
#include "Settings/settingcheckbox.h"
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
	void OnCommandFinished(Module::Common::RunCommand* module) override;
    void OnWaitTimeout() override;

    void OnUpdateSeedCalibrate();
    void OnUpdateContinueCalibrate();
    void OnContinueFrameChanged(int value);
    void OnOverallFrameChanged(int value);
    void OnCommandFlashbackValid(bool valid);

private: // types
    enum class State
    {
        TitleScreen,
        ContinueScreen,
        Flashback,
        CommandFlashback,
        CommandComplete,
    };

private: // function


private: // members
    Setting::SettingSpinBox* m_tid = Q_NULLPTR;
    Setting::SettingSpinBox* m_sid = Q_NULLPTR;
    Setting::SettingSpinBox* m_seedTime = Q_NULLPTR;
    Setting::SettingSpinBox* m_seedCalibrate = Q_NULLPTR;
    QLineEdit* m_seedHit = Q_NULLPTR;
    QPushButton* m_btnSeedUpdate = Q_NULLPTR;
    Setting::SettingSpinBox* m_continueFrames = Q_NULLPTR;
    QLabel* m_continueTime = Q_NULLPTR;
    Setting::SettingSpinBox* m_continueCalibrate = Q_NULLPTR;
    QLineEdit* m_continueHit = Q_NULLPTR;
    QPushButton* m_btnContinueUpdate = Q_NULLPTR;
    Setting::SettingSpinBox* m_overworldFrames = Q_NULLPTR;
    QLabel* m_overworldTime = Q_NULLPTR;
    Setting::SettingCheckBox* m_moveUp = Q_NULLPTR;
    Setting::System::SettingCommand* m_commandFlashback = Q_NULLPTR;
    Setting::System::SettingCommand* m_commandComplete = Q_NULLPTR;
    bool m_commandFlashbackFits = false;

    State m_state;
};
}

#endif // FRLG_RNGMANIPULATION_H
