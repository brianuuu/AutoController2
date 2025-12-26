#ifndef DEVFRAMECAPTURE_H
#define DEVFRAMECAPTURE_H

#include "../programbase.h"

namespace Program::Development
{
class DevFrameCapture : public ProgramBase
{
    Q_OBJECT
public:
    explicit DevFrameCapture(QObject* parent = nullptr);

    static QString GetCategory() { return "Development"; }
    static QString GetName() { return "Frame Capture"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    QString GetInternalName() const override { return "Dev-FrameCapture"; }
    QString GetDescription() const override {
        return "Test and create params for FrameCapture modules";
    }

    bool RequireSerial() const override { return false; }
    bool RequireVideo() const override { return true; }
    bool RequireAudio() const override { return false; }

    bool CanControlWhileRunning() const override { return true; }
    bool CanEditWhileRunning() const override { return true; }

    void Start() override;
    void Stop() override;

private:

};
}

#endif // DEVFRAMECAPTURE_H
