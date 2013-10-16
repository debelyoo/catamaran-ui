#include "sensortypemanager.h"

SensorTypeManager *SensorTypeManager::s_instance = NULL;

SensorTypeManager *SensorTypeManager::instance()
{
    if(!s_instance){
        s_instance = new SensorTypeManager();
    }
    return s_instance;
}

const QStringList &SensorTypeManager::list() const
{
    return m_types.keys();
}

const SensorType *SensorTypeManager::type(const QString &name) const
{
    if(m_types.contains(name)){
        return m_types[name];
    }
    return NULL;
}

bool SensorTypeManager::createType(const QString &name, const QString &dbTableName, int dbColumnCount)
{
    if(m_types.contains(name)){
        return false;
    }
    SensorType *s = new SensorType(name, dbTableName);
    m_types.insert(name, s);
    return true;
}

bool SensorTypeManager::deleteType(const QString &name)
{
    if(m_types.contains(name)){
        SensorType *s = m_types[name];
        m_types.remove(name);
        delete s;
        return true;
    }
    return false;
}

bool SensorTypeManager::updateType(const QString &name, const QString &dbTableName, int dbColumnCount)
{
    if(deleteType(name)){
        return createType(name, dbTableName, dbColumnCount);
    }
    return false;
}

SensorTypeManager::SensorTypeManager():
    m_types()
{
    m_types.insert("Unknown", new SensorType("Unknown", ""));
}
