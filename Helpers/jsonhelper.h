#ifndef JSONHELPER_H
#define JSONHELPER_H

#include <QFile>
#include <QJsonObject>
#include <QString>

#define SETTINGS_FILE "../UserSettings.json"

namespace JsonHelper
{
    QJsonObject ReadJson(QString const& path);
    void WriteJson(QString const& path, QJsonObject& object);

    QJsonObject ReadSetting(QString const& key);
    void WriteSetting(QString const& key, QJsonObject& object);

    QJsonObject ReadObject(QJsonObject const& object, QString const& key);
    bool ReadValue(QJsonObject const& object, QString const& key, QVariant& value, QVariant defaultValue = QVariant());
};

#endif // JSONHELPER_H
