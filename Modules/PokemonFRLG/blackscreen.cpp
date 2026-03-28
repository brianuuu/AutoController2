#include "blackscreen.h"

namespace Module::PokemonFRLG
{

BlackScreen::BlackScreen(QObject *parent)
    : SubModuleBase{parent}
{
    m_top = m_moduleHolder->AddFrameCapture("FRLG_EncounterTop");
    m_bottom = m_moduleHolder->AddFrameCapture("FRLG_EncounterBottom");
    m_blackTop = false;
    m_blackBottom = false;
}

void BlackScreen::OnFrameCaptureMatched(Common::FrameCapture *module, bool matched)
{
    if (OnModuleErrorQuit(module)) return;

    if (module == m_top)
    {
        m_blackTop = module->GetResultMatched();
    }
    else if (module == m_bottom)
    {
        m_blackBottom = module->GetResultMatched();
    }

    if (m_blackTop && m_blackBottom)
    {
        m_result = 0;
    }
    else if (!m_blackTop && !m_blackBottom)
    {
        m_result = 1;
    }
    else
    {
        m_result = 2;
    }

    emit notifyResult(this, m_result);
}

}
