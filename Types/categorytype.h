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
    default:                return "Unknown";
    }
}

#endif // CATEGORYTYPE_H
