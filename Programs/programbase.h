#ifndef PROGRAMBASE_H
#define PROGRAMBASE_H

#include <QLayout>
#include <QObject>

class ProgramBase : public QObject
{
    Q_OBJECT
public:
    explicit ProgramBase(QObject *parent = nullptr) : QObject{parent} {}

    virtual void PopulateSettings(QLayout* layout) = 0;
    virtual void LoadSettings() = 0;
    virtual void SaveSettings() const = 0;
};

#endif // PROGRAMBASE_H
