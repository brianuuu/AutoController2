#ifndef MANAGERCOLLECTION_H
#define MANAGERCOLLECTION_H

#include <QMap>
#include <QObject>

class ManagerCollection
{
private:
    static ManagerCollection& instance()
    {
        static ManagerCollection instance;
        return instance;
    }

private:
    QMap<QString, QObject*> m_managers;

public:
    template <class T>
    static T* AddManager(QWidget* parent = nullptr)
    {
        T* manager = new T(parent);
        instance().m_managers.insert(T::GetTypeID(), manager);
        return manager;
    }

    template <class T>
    static T* GetManager()
    {
        return qobject_cast<T*>(instance().m_managers.value(T::GetTypeID()));
    }
};

#endif // MANAGERCOLLECTION_H
