#ifndef BLACKSCREEN_H
#define BLACKSCREEN_H

#include "../submodulebase.h"

namespace Module::PokemonFRLG
{

class BlackScreen : public SubModuleBase
{
    Q_OBJECT
public:
    explicit BlackScreen(QObject *parent = nullptr);

    // from ModuleBase
    QString GetName() const override { return "FRLG-BlackScreen"; }

private slots:
    void OnFrameCaptureMatched(Module::Common::FrameCapture* module, bool matched) override;

private:
    Module::Common::FrameCapture* m_top = Q_NULLPTR;
    Module::Common::FrameCapture* m_bottom = Q_NULLPTR;
    bool m_blackTop = false;
    bool m_blackBottom = false;
};

}

#endif // BLACKSCREEN_H
