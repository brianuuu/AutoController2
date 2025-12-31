#ifndef OCRENTRYDATABASE_H
#define OCRENTRYDATABASE_H

#include <QMap>
#include <QString>
#include "Types/languagetype.h"

using OCREntries = std::map<QString, QStringList>;
using LanguageEntries = std::map<LanguageType, OCREntries>;
using EntryDatabase = std::map<QString, LanguageEntries>;

class OCREntryDatabase
{
private:
    static OCREntryDatabase& instance();
    static EntryDatabase& GetDatabase();

public:
    static bool EnsureDatabase(QString const& path);
    static bool EnsureDatabase(QString const& path, LanguageType language);
    static OCREntries const& GetEntries(QString const& path, LanguageType language);
    static QString GetNoDatabaseError(QString const& path, LanguageType language);
    static QString GetExtension() { return ".entries"; }

    // OCR matching utils
    static QString NormalizeString(QString const& str);
    static QString RemoveNonAlphaNumeric(QString const& str);
    static int GetLevenshteinDistance(QString const& a, QString const& b);
    static int GetLevenshteinDistanceSubString(QString const& longStr, QString const& shortStr);
    static int MatchSubStrings(QString const& query, QStringList const& subStrings, int* o_dist = nullptr);

private:
    // Key = path + name (without ../Resource or extension)
    // Example: Pokemon/Pokeballs etc.
    EntryDatabase m_database;
};

#endif // OCRENTRYDATABASE_H
