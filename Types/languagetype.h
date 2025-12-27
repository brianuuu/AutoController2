#ifndef LANGUAGETYPE_H
#define LANGUAGETYPE_H

#include <QString>
#include <QStringList>

enum LanguageType
{
    LT_English,
    LT_ChineseSimplified,
    LT_ChineseTraditional,
    LT_French,
    LT_German,
    LT_Italian,
    LT_Japanese,
    LT_Korean,
    LT_Spanish,

    LT_COUNT
};

static QString LanguageToString(LanguageType type)
{
    switch (type)
    {
    case LT_English:            return "English";
    case LT_ChineseSimplified:  return "Chinese (Simplified)";
    case LT_ChineseTraditional: return "Chinese (Traditional)";
    case LT_French:             return "French";
    case LT_German:             return "German";
    case LT_Italian:            return "Italian";
    case LT_Japanese:           return "Japanese";
    case LT_Korean:             return "Korean";
    case LT_Spanish:            return "Spanish";
    default:                    return "Unknown";
    }
}

static QStringList LanguageStringList()
{
    QStringList list;
    for (int i = 0; i < LT_COUNT; i++)
    {
        list << LanguageToString((LanguageType)i);
    }
    return list;
}

static QString LanguageToPrefix(LanguageType type)
{
    switch (type)
    {
    case LT_English:            return "eng";
    case LT_ChineseSimplified:  return "chi_sim";
    case LT_ChineseTraditional: return "chi_tra";
    case LT_French:             return "fra";
    case LT_German:             return "deu";
    case LT_Italian:            return "ita";
    case LT_Japanese:           return "jpn";
    case LT_Korean:             return "kor";
    case LT_Spanish:            return "spa";
    default:                    return "invalid";
    }
}

#endif // LANGUAGETYPE_H
