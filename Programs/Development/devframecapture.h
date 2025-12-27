#ifndef DEVFRAMECAPTURE_H
#define DEVFRAMECAPTURE_H

#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>

#include "../programbase.h"
#include "Programs/Modules/Common/framecapture.h"
#include "Programs/Settings/settingcolor.h"
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
    void OnListChanged(QString const& str);
    void OnModeChanged(int mode);

    void OnLeftChanged(int value);
    void OnTopChanged(int value);
    void OnWidthChanged(int value);
    void OnHeightChanged(int value);
    void OnRangeChanged();
    void OnColorChanged(QColor color);

    void OnMousePressed(QPoint pos);
    void OnMouseMoved(QPoint pos);

    void OnSave();
    void OnDelete();
    void OnOpenDirectory();

private:
    QPoint GetPoint() const;
    QRect GetRect() const;
    HsvRange GetRange() const;

    void UpdateSettingEnabled();
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

    Setting::SettingColor* m_color = Q_NULLPTR;

    QPushButton* m_btnSave = Q_NULLPTR;
    QPushButton* m_btnDelete = Q_NULLPTR;
    QPushButton* m_btnDirectory = Q_NULLPTR;

    Module::Common::FrameCapture* m_moduleCapture = Q_NULLPTR;
};
}

#endif // DEVFRAMECAPTURE_H
