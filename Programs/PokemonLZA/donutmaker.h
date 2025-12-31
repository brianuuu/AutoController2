#ifndef DONUTMAKER_H
#define DONUTMAKER_H

#include "../programbase.h"
#include "Programs/Settings/settinglineedit.h"
#include "Programs/Settings/settingspinbox.h"
#include "Programs/Settings/PokemonLZA/settingflavorpower.h"

namespace Program::PokemonLZA
{
class DonutMaker : public ProgramBase
{
    Q_OBJECT
public:
    explicit DonutMaker(QObject* parent = nullptr);

    static QString GetCategory() { return "Pokemon Legends: Z-A"; }
    static QString GetName() { return "Donut Maker"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    QString GetInternalName() const override { return "PLZA-DonutMaker"; }
    QString GetDescription() const override {
        return "Automatically make donuts with specific flavor powers";
    }

    bool RequireSerial() const override { return true; }
    bool RequireVideo() const override { return true; }
    bool RequireAudio() const override { return false; }

    bool CanRun() const override;

    void Start() override;
    void Stop() override;

private:
    enum State
    {
        Restart,
        TitleScreen,
        GameLoadStart,
        GameLoadWait,

        BackupSave,
        FlyToHotelZ,
        FlyToHotelZLoadWait,
        EnterHotelZ,
        EnterHotelZLoadWait,

        TalkToAnsha,
        SelectBerries,
        MakeDonut,
        PowerCapture,
        QuitDonut,

        FlyToVertPC,
        FlyToVertPCLoadWait,
        WalkToNurseJoy,
    };
    Q_ENUM(State)

private slots:
    void OnOrderChanged(QString const& str);
    void OnCommandFinished();
    void OnFrameCaptureMatched(bool matched);
    void OnOCRFinished();

private:
    void VerifyOrder();

    // states
    void StateFlyToVertPC();
    void StateBackupSave();
    void StateRestart();
    void AddBlackScreenModule();

private:
    Setting::SettingSpinBox* m_count = Q_NULLPTR;
    Setting::SettingLineEdit* m_order = Q_NULLPTR;
    QLabel* m_status = Q_NULLPTR;

    using FlavorPower = Setting::PokemonLZA::SettingFlavorPower;
    FlavorPower* m_power = Q_NULLPTR;
    FlavorPower::PowerSlots m_cachedSlots;

    State m_state;
    bool m_validOrder = false;

    QList<QString> m_powerEntries;
    int m_donutCount = 0;
};
}

#endif // DONUTMAKER_H
