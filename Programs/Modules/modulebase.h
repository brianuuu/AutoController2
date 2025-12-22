#ifndef MODULEBASE_H
#define MODULEBASE_H

#include <QThread>

#include "Enums/system.h"

namespace Module
{
class ModuleBase : public QThread
{
    Q_OBJECT

public:
    explicit ModuleBase(QObject *parent = nullptr);

    virtual QString GetName() const = 0;

    // should only be accessed when module is finished
    int GetResult() const { return m_result; }

signals:
    void notifyLog(QString const& category, QString const& log, LogType type = LOG_Normal) const;

protected slots:
    virtual void OnStarted() const;
    virtual void OnFinished() const;

protected:
    void PrintLog(QString const& log, LogType type = LOG_Normal) const;

protected:
    int m_result = -1;
};
}

#endif // MODULEBASE_H
