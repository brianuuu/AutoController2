#ifndef SETTINGFLAVORPOWER_H
#define SETTINGFLAVORPOWER_H

#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QRegularExpressionValidator>

#include "../settingbase.h"

namespace Setting::PokemonLZA {

class SettingFlavorPower : public QWidget, public SettingBase
{
   Q_OBJECT
public:
    explicit SettingFlavorPower(QString const& name);

    // from SettingBase
    void Load(QJsonObject &object) override;
    void Save(QJsonObject &object) const override;
    void ResetDefault() override;

    // query
    QList<QStringList> GetPowerSlots() const;

private slots:
    void OnFilterChanged();

private:
    void ResetLists();

private:
    QListWidget* m_power[3] = {Q_NULLPTR, Q_NULLPTR, Q_NULLPTR};
    QListWidget* m_allPower = Q_NULLPTR;
    QLineEdit* m_filter = Q_NULLPTR;
};
}

#endif // SETTINGFLAVORPOWER_H
