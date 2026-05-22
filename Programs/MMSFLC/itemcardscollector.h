#ifndef MMSFLC_ITEMCARDSCOLLECTOR_H
#define MMSFLC_ITEMCARDSCOLLECTOR_H

#include "../programbase.h"
#include "Settings/settingcheckbox.h"
#include "Types/categorytype.h"

namespace Program::MMSFLC
{
class ItemCardsCollector : public ProgramBase
{
public:
    explicit ItemCardsCollector(QObject *parent = nullptr) : ProgramBase{parent} {}

    static CategoryType GetCategory() { return CT_MMSFLC; }
    static QString GetName() { return "MMSF2: Item Cards Collector"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    QString GetInternalName() const override { return "MMSFLC-ItemCardsCollector"; }
    QString GetDescription() const override {
        return "Auto collecting all Item Cards in Wave Command Cards for MMSF2\nStart the program at Omega-Xis screen";
    }

    bool RequireSerial() const override { return true; }
    bool RequireVideo() const override { return !m_macroOnly->isChecked(); }
    bool RequireAudio() const override { return false; }

    void Start() override;
    void Stop() override;

private slots:
	void OnCommandFinished(Module::Common::RunCommand* module) override;
    void OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched) override;

private: // types
    enum class State
    {
        ToItemCards,
        Select,
        Collect,
    };

private: // function
    void StateStart();

private: // members
    Setting::SettingCheckBox* m_macroOnly = Q_NULLPTR;

	State m_state;
    int m_index = 0;
};
}

#endif // MMSFLC_ITEMCARDSCOLLECTOR_H
