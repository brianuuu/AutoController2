#include "permutationbase.h"

namespace Program::PokemonFRLG {

PermutationBase::PermutationBase(QObject *parent)
    : ProgramBase{parent}
{

}

void PermutationBase::PopulateSettings(QBoxLayout *layout)
{
    m_seedFrame = new Setting::SettingSpinBox("SeedFrame", 0, 9999);
    m_savedSettings.insert(m_seedFrame);
    AddSetting(layout, "Seed Frame:", "Wait this many frames (not accurate) before pressing A at the title screen. This setting is changed by the program while running", m_seedFrame, true);

    m_advanceFrame = new Setting::SettingSpinBox("AdvanceFrame", 0, 9999);
    m_savedSettings.insert(m_advanceFrame);
    AddSetting(layout, "Advance Frame:", "Wait this many frames (not accurate) before finishing dialogue after picking starter. This setting is changed by the program while running", m_advanceFrame, true);
}

void PermutationBase::RegisterStats()
{
    RegisterStat(m_statReset, "Resets");
    RegisterStat(m_statShiny, "Shinies");
}

void PermutationBase::Start()
{
    ProgramBase::Start();
    m_currentMaxFrame = qMax(m_seedFrame->value(), m_advanceFrame->value());
}

void PermutationBase::Stop()
{
    ProgramBase::Stop();
}

void PermutationBase::GetNextPermutation()
{
    // Order: 0-0, 1-0, 0-1, 2-0, 2-1, 2-2, 1-2, 0-2, 3-0, 3-1, 3-2, 3-3, 2-3, 1-3, 0-3, 4-0, etc.
    if (m_advanceFrame->value() == m_currentMaxFrame)
    {
        if (m_seedFrame->value() == 0)
        {
            ++m_currentMaxFrame;
            m_seedFrame->setValue(m_currentMaxFrame);
            m_advanceFrame->setValue(0);
        }
        else
        {
            m_seedFrame->setValue(m_seedFrame->value() - 1);
        }
    }
    else
    {
        m_advanceFrame->setValue(m_advanceFrame->value() + 1);
    }
}

}
