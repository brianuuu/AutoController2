#ifndef SETTINGBASE_H
#define SETTINGBASE_H

#include <QJsonObject>
#include <QObject>

class SettingBase
{
public:
    SettingBase(QString const& name) : m_name(name) {}

    virtual void Load(QJsonObject& object) = 0;
    virtual void Save(QJsonObject& object) const = 0;

public:
    QString m_name; // for save & load reference
};

#endif // SETTINGBASE_H
