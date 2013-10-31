#include "sensortypemanager.h"

SensorTypeManager *SensorTypeManager::s_instance = NULL;

SensorTypeManager *SensorTypeManager::instance()
{
    if(!s_instance){
        s_instance = new SensorTypeManager();
    }
    return s_instance;
}

const QStringList SensorTypeManager::list() const
{
    return m_types.keys();
}

const QStringList &SensorTypeManager::sortedList() const
{
    return m_sortedList;
}

const SensorType *SensorTypeManager::type(const QString &name) const
{
    if(m_types.contains(name)){
        return m_types[name];
    }
    return NULL;
}

bool SensorTypeManager::createType(const QString &name)
{
    if(m_types.contains(name)){
        return false;
    }
    SensorType *s = new SensorType(name);
    m_types.insert(name, s);
    updateSortedList();
    return true;
}

bool SensorTypeManager::deleteType(const QString &name)
{
    if(m_types.contains(name)){
        SensorType *s = m_types[name];
        m_types.remove(name);
        delete s;
        updateSortedList();
        return true;
    }
    return false;
}

bool SensorTypeManager::updateType(const QString &name)
{
    if(deleteType(name)){
        return createType(name);
    }
    return false;
}

SensorTypeManager::SensorTypeManager():
    m_types(),
    m_sortedList()
{
    m_types.insert("Unknown", new SensorType("Unknown"));
    updateSortedList();
}

void SensorTypeManager::updateSortedList()
{
    m_sortedList.clear();
    m_sortedList.append("Unknown");
    foreach (const QString &s, m_types.keys()) {
        if(s != "Unknown"){
            m_sortedList.append(s);
        }
    }
}


QDataStream &operator<<(QDataStream &stream, const SensorTypeManager &a)
{

    QList<SensorType *> list;
    foreach(SensorType *st, a.m_types.values()){
        if(st->name() != "Unknown"){
            list.append(st);
        }
    }
    stream << list.count();
    foreach(const SensorType *st, list){
        stream << *st;
    }
    return stream;
}

QDataStream &operator>>(QDataStream &stream, SensorTypeManager &a)
{
    int n;
    stream >> n;
    for(int i=0;i<n;++i){
        QString name;
        stream >> name;
        a.createType(name);
    }
    return stream;
}
