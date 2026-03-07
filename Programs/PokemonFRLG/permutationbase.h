#ifndef FRLG_PERMUTATIONBASE_H
#define FRLG_PERMUTATIONBASE_H

#include <QBoxLayout>

#include "../programbase.h"
#include "Settings/settingspinbox.h"

namespace Program::PokemonFRLG {

class PermutationBase : public ProgramBase
{
public:
    explicit PermutationBase(QObject *parent = nullptr);

    void PopulateSettings(QBoxLayout* layout) override;
    void RegisterStats() override;

    bool RequireSerial() const override { return true; }
    bool RequireVideo() const override { return true; }
    bool RequireAudio() const override { return false; }

    bool ShouldLog() const override { return true; }
    bool BypassBorderCheck() const override { return true; }

    void Start() override;
    void Stop() override;

protected: // function
    void GetNextPermutation();

protected:
    Setting::SettingSpinBox* m_seedFrame = Q_NULLPTR;
    Setting::SettingSpinBox* m_advanceFrame = Q_NULLPTR;
    int m_currentMaxFrame = 0;

    Stat m_statReset;
    Stat m_statShiny;
};

}

#endif // FRLG_PERMUTATIONBASE_H
