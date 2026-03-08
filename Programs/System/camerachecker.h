#ifndef SYSTEM_CAMERACHECKER_H
#define SYSTEM_CAMERACHECKER_H

#include "../programbase.h"
#include "Types/categorytype.h"

namespace Program::System
{
class CameraChecker : public ProgramBase
{
public:
    explicit CameraChecker(QObject *parent = nullptr);

    static CategoryType GetCategory() { return CT_System; }
    static QString GetName() { return "Camera Checker"; }

    // from ProgramBase
    QString GetInternalName() const override { return "System-CameraChecker"; }
    QString GetDescription() const override {
        return "Detects Nintendo Switch system type and checks camera delay";
    }

    bool RequireSerial() const override { return true; }
    bool RequireVideo() const override { return true; }
    bool RequireAudio() const override { return false; }

    void Start() override;
    void Stop() override;

private: // types
    enum class State
    {
        DetectTheme,
        SystemSetting,
        ButtonMenu,
        ButtonTest,
        ReturnHome,
    };

    QColor const ThemeDark1 = QColor(44,44,44);
    QColor const ThemeDark2= QColor(27,27,27);
    QColor const ThemeLight = QColor(234,234,234);

private slots:
    void OnCommandFinished() override;
    void OnFrameCaptureMatched(bool matched) override;

private: // function
    void StateButtonTest();

private: // members
    QColor m_color;
    int m_button = 0;
    qint64 m_delay = 0;

    State m_state;
};
}

#endif // SYSTEM_CAMERACHECKER_H
