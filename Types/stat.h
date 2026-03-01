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

    int GetValue() const { return m_value; }
    bool GetHideZero() const { return m_hideZero; }
    QString GetString() const { return QString::number(m_value); }

    void ResetValue()
    {
        m_value = 0;
        emit notifyStatChanged();
    }

    void SetValue(int value)
    {
        m_value = value;
        emit notifyStatChanged();
    }

    void AddValue(int add)
    {
        m_value = qMax(0, m_value + add);
        emit notifyStatChanged();
    }

    Stat& operator++() // Prefix increment operator.
    {
        ++m_value;
        emit notifyStatChanged();
        return *this;
    }

signals:
    void notifyStatChanged();

private:
    QString m_name;
    int m_value = 0;
    bool m_hideZero = false;
};

#endif // STAT_H
