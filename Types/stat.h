#ifndef STAT_H
#define STAT_H

#include <QObject>

class Stat : public QObject
{
    Q_OBJECT
public:
    Stat() {}
    Stat(QString const& name, bool hideZero = false)
        : m_name(name), m_hideZero(hideZero)
    {}

    void SetName(QString const& name) { m_name = name; }
    QString GetName() const { return m_name; }

    void SetHideZero(bool hideZero) { m_hideZero = hideZero; }
    bool GetHideZero() const { return m_hideZero; }

    int GetValue() const { return m_value; }
    QString GetString() const { return QString::number(m_value); }

    int GetTotal() const { return m_total; }
    QString GetTotalString() const { return QString::number(m_total); }

    void Reset()
    {
        m_value = 0;
        m_total = 0;
        emit notifyStatChanged();
    }

    void SetValue(int value, bool signal = true)
    {
        m_value = value;
        if (signal)
        {
            emit notifyStatChanged();
        }
    }

    void SetTotal(int total, bool signal = true)
    {
        m_total = total;
        if (signal)
        {
            emit notifyStatChanged();
        }
    }

    Stat& operator++() // Prefix increment operator.
    {
        ++m_value;
        ++m_total;
        emit notifyStatChanged();
        return *this;
    }

signals:
    void notifyStatChanged();

private:
    QString m_name;
    int m_value = 0;
    int m_total = 0;
    bool m_hideZero = false;
};

#endif // STAT_H
