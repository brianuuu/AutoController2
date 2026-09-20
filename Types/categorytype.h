#ifndef CATEGORYTYPE_H
#define CATEGORYTYPE_H

#include <QString>
#include <QStringList>

enum CategoryType
{
    CT_Development,
    CT_System,
    CT_PLZA,
    CT_FRLG,
    CT_MMSFLC,
    CT_Raiders,

    CT_COUNT
};

static QString CategoryToString(CategoryType category)
{
    switch (category)
    {
    case CT_Development:    return "Development";
    case CT_System:         return "System";
    case CT_PLZA:           return "Pokemon Legends: Z-A";
    case CT_FRLG:           return "Pokemon Fire Red/Leaf Green";
    case CT_MMSFLC:         return "Mega Man Star Force Legacy Collection";
    case CT_Raiders:        return "Splatoon Raiders";
    default:                return "Unknown";
    }
}

static CategoryType StringToCategory(QString const& str)
{
    QString const lower = str.toLower();
    for (int i = 0; i < CT_COUNT; i++)
    {
        CategoryType type = (CategoryType)i;
        if (lower == CategoryToString(type).toLower())
        {
            return type;
        }
    }

    return CT_COUNT;
}

#endif // CATEGORYTYPE_H
