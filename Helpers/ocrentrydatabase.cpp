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
    //JsonHelper::WriteJson(RESOURCES_PATH + path + GetExtension(), object);

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

int OCREntryDatabase::GetLevenshteinDistance(const QString &a, const QString &b)
{
    QVector<int> v0(b.size() + 1);
    QVector<int> v1(b.size() + 1);

    for (int i = 0; i <= b.size(); i++)
    {
        v0[i] = i;
    }

    for (int i = 0; i < a.size(); i++)
    {
        v1[0] = i + 1;

        for (int j = 0; j < b.size(); j++)
        {
            int deletion = v0[j + 1] + 1;
            int insertion = v1[j] + 1;
            int substitution = v0[j];
            if (a[i] != b[j])
            {
                substitution += 1;
            }

            v1[j + 1] = qMin(deletion, qMin(insertion, substitution));
        }

        qSwap(v0, v1);
    }

    return v0[b.size()];
}

int OCREntryDatabase::GetLevenshteinDistanceSubString(const QString &longStr, const QString &shortStr)
{
    QVector<int> v0(shortStr.size() + 1);
    QVector<int> v1(shortStr.size() + 1);

    for (int i = 0; i <= shortStr.size(); i++)
    {
        v0[i] = i;
    }

    int min = shortStr.size();
    for (int i = 0; i < longStr.size(); i++)
    {
        v1[0] = 0;

        for (int j = 0; j < shortStr.size(); j++)
        {
            int deletion = v0[j + 1] + 1;
            int insertion = v1[j] + 1;
            int substitution = v0[j];
            if (longStr[i] != shortStr[j])
            {
                substitution += 1;
            }

            v1[j + 1] = qMin(deletion, qMin(insertion, substitution));
        }

        qSwap(v0, v1);
        min = qMin(min, v0[shortStr.size()]);
    }

    return min;
}

int OCREntryDatabase::MatchSubStrings(const QString &query, const QStringList &subStrings, int *o_dist)
{
    // Note: "query" and "subStrings" should be already normalized
    // This function finds the best match with in the entry,
    // though this doesn't matter much as any match in an entry always counts

    int minDist = INT_MAX;
    int minSubStringID = -1;
    for (int i = 0; i < subStrings.size(); i++)
    {
        QString const& subString = subStrings[i];

        // check for exact match
        if (query == subString)
        {
            minDist = 0;
            minSubStringID = i;
            break;
        }

        // Calculate Levenshtein Distance
        int const dist = GetLevenshteinDistance(query, subString);

        // Pretty naive way to filter, for modification less than database str/2
        if (dist < minDist && dist <= subString.size() / 2 )
        {
            minDist = dist;
            minSubStringID = i;
        }
    }

    if (minSubStringID >= 0 && o_dist)
    {
        *o_dist = minDist;
    }

    return minSubStringID;
}
