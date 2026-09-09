#ifndef AUTOTEMPRAHDEN_H
#define AUTOTEMPRAHDEN_H

#include "../programbase.h"
#include "Settings/settingspinbox.h"
#include "Types/categorytype.h"

namespace Program::Raiders
{
class AutoTemprahDen : public ProgramBase
{
public:
    explicit AutoTemprahDen(QObject *parent = nullptr) : ProgramBase{parent} {}

    static CategoryType GetCategory() { return CT_Raiders; }
    static QString GetName() { return "Auto Temprah Den"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    void RegisterStats() override;
    QString GetInternalName() const override { return "Raiders-AutoTemprahDen"; }
    QString GetDescription() const override {
        return "Auto complete Temprah Den raid";
    }

    bool RequireSerial() const override { return true; }
    bool RequireVideo() const override { return true; }
    bool RequireAudio() const override { return false; }

    void Start() override;
    void Stop() override;

private slots:
	void OnCommandFinished(Module::Common::RunCommand* module) override;
    void OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched) override;
    void OnWaitTimeout() override;

private: // types
    enum class State
    {
        StartRaid,
        AutoFire,
        FinishRaid,
        CollectLoot,
    };

private: // function
    // states
    void StateStartRaid();
    void StateFinishRaid(bool failed);

private: // members
    Setting::SettingSpinBox* m_count = Q_NULLPTR;
    Module::Common::FrameCapture* m_fail = Q_NULLPTR;

	State m_state;
    int m_raidCount = 0;
    bool m_blackDetected = false;

    Stat m_statRaids;
    Stat m_statSuccess;
};
}

#endif // AUTOTEMPRAHDEN_H
