#ifndef MODULEBASE_H
#define MODULEBASE_H

#include <QThread>

#include "Types/logtype.h"

namespace Module
{
class ModuleBase : public QThread
{
    Q_OBJECT
public:
    explicit ModuleBase(QObject *parent = nullptr);

    virtual QString GetName() const = 0;
    uint GetID() const { return m_id; }

    // try stopping module, not thread safe
    virtual void stop() { m_terminate = true; quit(); }

    // should only be accessed when module is finished
    int GetResult() const { return m_result; }
    QString GetError() const { return m_error; }

signals:
    void notifyLog(QString const& category, QString const& log, LogType type = LOG_Normal) const;

protected slots:
    virtual void OnStarted() const;
    virtual void OnFinished() const;

protected:
    void PrintLog(QString const& log, LogType type = LOG_Normal) const;

protected:
    uint m_id = 0;
    std::atomic_bool m_terminate = false;
    int m_result = 0;
    QString m_error;
};
}

#endif // MODULEBASE_H
