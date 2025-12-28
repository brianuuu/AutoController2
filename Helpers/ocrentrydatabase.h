#ifndef OCRENTRYDATABASE_H
#define OCRENTRYDATABASE_H

#include <QMap>
#include <QString>
#include "Types/languagetype.h"

using OCREntries = QMap<QString, QStringList>;
using LanguageEntries = QMap<LanguageType, OCREntries>;
using EntryDatabase = QMap<QString, LanguageEntries>;

class OCREntryDatabase
{
private:
    static OCREntryDatabase& instance();
    static EntryDatabase& GetDatabase();

public:
    static bool EnsureDatabase(QString const& path);
    static OCREntries const& GetEntries(QString const& path, LanguageType language);
    static QString GetExtension() { return ".entries"; }

    // Fix strings
    static QString NormalizeString(QString const& str);
    static QString RemoveNonAlphaNumeric(QString const& str);

private:
    // Key = path + name (without ../Resource or extension)
    // Example: Pokemon/Pokeballs etc.
    EntryDatabase m_database;
};

#endif // OCRENTRYDATABASE_H
