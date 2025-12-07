#ifndef PROGRAMBASE_H
#define PROGRAMBASE_H

#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QObject>

#include "Programs/Settings/settingcombobox.h"

class ProgramBase : public QObject
{
    Q_OBJECT
public:
    explicit ProgramBase(QObject *parent = nullptr) : QObject{parent} {}

    void LoadSettings();
    void SaveSettings() const;

    virtual void PopulateSettings(QBoxLayout* layout) = 0;
    virtual QString GetInternalName() const = 0;
    virtual QString GetDescription() const = 0;

    bool HasSettings() { return !m_settings.empty(); }
    void ResetDefault();

protected:
    void AddSingleItem
    (
        QBoxLayout *layout,
        QString const& name,
        QString const& description,
        QWidget* setting
    );

    SettingComboBox* AddComboBox
    (
        QBoxLayout *layout,
        QString const& name,
        QString const& description,
        QString const& settingName,
        QStringList const& list
    );

private:
    QList<SettingBase*> m_settings;
};

#endif // PROGRAMBASE_H
