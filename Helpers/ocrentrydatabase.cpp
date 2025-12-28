#include "ocrentrydatabase.h"

#include "Helpers/jsonhelper.h"
#include "defines.h"

OCREntryDatabase& OCREntryDatabase::instance()
{
    static OCREntryDatabase database;
    return database;
}

EntryDatabase &OCREntryDatabase::GetDatabase()
{
    return instance().m_database;
}

bool OCREntryDatabase::EnsureDatabase(const QString &path)
{
    // check if database already exist
    EntryDatabase& database = GetDatabase();
    if (database.contains(path)) return true;

    // read database
    QJsonObject object = JsonHelper::ReadJson(RESOURCES_PATH + path + GetExtension());
    if (object.isEmpty()) return false;

    LanguageEntries languageEntries;
    for (int i = 0; i < LT_COUNT; i++)
    {
        LanguageType const language = LanguageType(i);
        QString const prefix = LanguageToPrefix(language);

        QJsonObject languageObject = JsonHelper::ReadObject(object, prefix);
        if (languageObject.isEmpty()) continue;

        // Read all entries from json file
        //qDebug() << LanguageToString(language);
        OCREntries entries;
        for (auto it = languageObject.begin(); it != languageObject.end(); ++it)
        {
            //qDebug() << it.key();
            QStringList valueList;
            for (QJsonValueRef value : it.value().toArray())
            {
                //qDebug() << value.toString();
                valueList.push_back(NormalizeString(value.toString()));
            }
            entries.insert(it.key(), valueList);
        }
        languageEntries.insert(language, entries);
    }
    database.insert(path, languageEntries);
    return true;
}

const OCREntries &OCREntryDatabase::GetEntries(const QString &path, LanguageType language)
{
    EntryDatabase const& database = GetDatabase();
    if (database.contains(path))
    {
        LanguageEntries const& languageEntries = database[path];
        if (languageEntries.contains(language))
        {
            return GetDatabase()[path][language];
        }
    }

    static OCREntries emptyEntries;
    return emptyEntries;
}

QString OCREntryDatabase::NormalizeString(const QString &str)
{
    QString temp = str.normalized(QString::NormalizationForm_KD);
    temp = RemoveNonAlphaNumeric(temp);
    return temp.toLower();
}

QString OCREntryDatabase::RemoveNonAlphaNumeric(const QString &str)
{
    QString temp;
    for (QChar c : str)
    {
        if (c.isLetterOrNumber()
            || c == QChar(0x3099)  // Japanese dakuten
            || c == QChar(0x309A)) // Japanese handakuten
        {
            temp += c;
            continue;
        }
    }

    return temp;
}
