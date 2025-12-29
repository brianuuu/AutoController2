#ifndef DONUTMAKER_H
#define DONUTMAKER_H

#include "../programbase.h"
#include "Programs/Settings/settinglineedit.h"
#include "Programs/Settings/settingspinbox.h"

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

private slots:
    void OnOrderChanged(QString const& str);

private:
    void VerifyOrder();

private:
    Setting::SettingSpinBox* m_count = Q_NULLPTR;
    Setting::SettingLineEdit* m_order = Q_NULLPTR;
    QLabel* m_status = Q_NULLPTR;

    bool m_validOrder = false;
};
}

#endif // DONUTMAKER_H
