#ifndef PLZA_DONUTMAKER_H
#define PLZA_DONUTMAKER_H

#include "../programbase.h"
#include "Settings/settinglineedit.h"
#include "Settings/settingspinbox.h"
#include "Settings/PokemonLZA/settingflavorpower.h"
#include "Types/categorytype.h"

namespace Program::PokemonLZA
{
class DonutMaker : public ProgramBase
{
    Q_OBJECT
public:
    explicit DonutMaker(QObject* parent = nullptr);

    static CategoryType GetCategory() { return CT_PLZA; }
    static QString GetName() { return "Donut Maker"; }

    // from ProgramBase
    void PopulateSettings(QBoxLayout* layout) override;
    void RegisterStats() override;
    QString GetInternalName() const override { return "PLZA-DonutMaker"; }
    QString GetDescription() const override {
        return "Make donuts and reset until one or multiple of the same donut with specified flavor power is found";
    }

    bool RequireSerial() const override { return true; }
    bool RequireVideo() const override { return true; }
    bool RequireAudio() const override { return false; }

    bool ShouldLog() const override { return true; }
    bool CanRun() const override;

    void Start() override;
    void Stop() override;

private:
    enum class State
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

private slots:
    void OnOrderChanged(QString const& str);
    void OnCommandFinished() override;
    void OnFrameCaptureMatched(bool matched) override;
    void OnOCRFinished();

private:
    void VerifyOrder();

    // states
    void StateFlyToVertPC();
    void StateBackupSave();
    void StateRestart();

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

    Stat m_statMade;
    Stat m_statFound;
};
}

#endif // PLZA_DONUTMAKER_H
