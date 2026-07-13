#ifndef AUTOBATTLERSF3_H
#define AUTOBATTLERSF3_H

#include "../programbase.h"
#include "Types/categorytype.h"

namespace Program::MMSFLC
{
class AutoBattlerSF3 : public ProgramBase
{
public:
    explicit AutoBattlerSF3(QObject *parent = nullptr) : ProgramBase{parent} {}

    static CategoryType GetCategory() { return CT_MMSFLC; }
    static QString GetName() { return "Auto Battler (MMSF3)"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    void RegisterStats() override;
    QString GetInternalName() const override { return "MMSFLC-AutoBattlerSF3"; }
    QString GetDescription() const override {
        return "Automatically grinds battle in MMSF3. For collecting Noise abilities, Noise Frags, Zenny etc.";
    }

    bool RequireSerial() const override { return true; }
    bool RequireVideo() const override { return true; }
    bool RequireAudio() const override { return false; }

    void Start() override;
    void Stop() override;

private slots:
	void OnCommandFinished(Module::Common::RunCommand* module) override;
    void OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched) override;

private: // types
    enum class State
    {
        Encounter,
        UseDefaultCard,
        EndBattle,
        NoiseChange,
    };

private: // function
    void StateStart();
    void StateEndBattle();

private: // members
    Module::Common::FrameCapture* m_top = Q_NULLPTR;
    Module::Common::FrameCapture* m_bottom = Q_NULLPTR;

	State m_state;
    Stat m_statBattles;
    int m_count = 0;
};
}

#endif // AUTOBATTLERSF3_H
