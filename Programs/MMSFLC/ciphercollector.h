#ifndef MMSFLC_CIPHERCOLLECTOR_H
#define MMSFLC_CIPHERCOLLECTOR_H

#include "../programbase.h"
#include "Settings/settingcheckbox.h"
#include "Settings/settingcombobox.h"
#include "Types/categorytype.h"

namespace Program::MMSFLC
{
class CipherCollector : public ProgramBase
{
public:
    explicit CipherCollector(QObject *parent = nullptr) : ProgramBase{parent} {}

    static CategoryType GetCategory() { return CT_MMSFLC; }
    static QString GetName() { return "Cipher Collector"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    QString GetInternalName() const override { return "MMSFLC-CipherCollector"; }
    QString GetDescription() const override {
        return "Auto collecting all Cipher rewards for MMSF1 or MMSF2\nStart the program in the overworld";
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
        ToCipherList,
        Select,
        Collect,
    };

private: // function
    void StateStart();
    QString GetDelayCommand() const;

private: // members
    Setting::SettingComboBox* m_game = Q_NULLPTR;
    Setting::SettingCheckBox* m_macroOnly = Q_NULLPTR;

	State m_state;
    int m_index = 0;
};
}

#endif // MMSFLC_CIPHERCOLLECTOR_H
