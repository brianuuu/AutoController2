#include "jsonhelper.h"

QJsonObject JsonHelper::ReadJson(const QString &path)
{
    QFile file(path);
    if (file.open(QFile::ReadOnly))
    {
        QJsonParseError error;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(file.readAll(), &error);
        if (error.error == QJsonParseError::NoError)
        {
            return jsonDocument.object();
        }
    }

    return QJsonObject();
}

void JsonHelper::WriteJson(const QString &path, QJsonObject &object)
{
    QJsonDocument doc(object);

    QFile file(path);
    if (file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
    {
        file.write(doc.toJson());
        file.close();
    }
}

QJsonObject JsonHelper::ReadSetting(const QString &key)
{
    QJsonObject settings = ReadJson(SETTINGS_FILE);
    return ReadObject(settings, key);
}

void JsonHelper::WriteSetting(const QString &key, QJsonObject &object)
{
    QJsonObject settings = ReadJson(SETTINGS_FILE);
    settings.insert(key, object);
    WriteJson(SETTINGS_FILE, settings);
}

QJsonObject JsonHelper::ReadObject(const QJsonObject &object, const QString &key)
{
    for (auto it = object.begin(); it != object.end(); ++it)
    {
        if (it.key() == key)
        {
            return it.value().toObject();
        }
    }

    return QJsonObject();
}

bool JsonHelper::ReadValue(const QJsonObject &object, const QString &key, QVariant &value, QVariant defaultValue)
{
    for (auto it = object.begin(); it != object.end(); ++it)
    {
        if (it.key() == key)
        {
            value = it.value().toVariant();
            return true;
        }
    }

    value = defaultValue;
    return false;
}
