#ifndef DEVFRAMECAPTURE_H
#define DEVFRAMECAPTURE_H

#include "../programbase.h"
#include "Programs/Modules/Common/framecapture.h"
#include "Programs/Settings/settingcombobox.h"
#include "Programs/Settings/settingspinbox.h"

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

private slots:
    void OnModeChanged(int mode);

    void OnLeftChanged(int value);
    void OnTopChanged(int value);
    void OnWidthChanged(int value);
    void OnHeightChanged(int value);
    void OnRangeChanged();

private:
    QPoint GetPoint() const;
    QRect GetRect() const;
    HsvRange GetRange() const;

    void UpdateRect();
    void UpdateRange();

private:
    Setting::SettingComboBox* m_list = Q_NULLPTR;
    Setting::SettingComboBox* m_mode = Q_NULLPTR;

    Setting::SettingSpinBox* m_left = Q_NULLPTR;
    Setting::SettingSpinBox* m_top = Q_NULLPTR;
    Setting::SettingSpinBox* m_width = Q_NULLPTR;
    Setting::SettingSpinBox* m_height = Q_NULLPTR;

    Setting::SettingSpinBox* m_minH = Q_NULLPTR;
    Setting::SettingSpinBox* m_minS = Q_NULLPTR;
    Setting::SettingSpinBox* m_minV = Q_NULLPTR;
    Setting::SettingSpinBox* m_maxH = Q_NULLPTR;
    Setting::SettingSpinBox* m_maxS = Q_NULLPTR;
    Setting::SettingSpinBox* m_maxV = Q_NULLPTR;

    Module::Common::FrameCapture* m_moduleCapture = Q_NULLPTR;
};
}

#endif // DEVFRAMECAPTURE_H
