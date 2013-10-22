#include "sensorType.h"

SensorType::SensorType(int i, QString n, QString tn, QString dn, int cCount):
    id(i),
    m_name(n),
    m_dbTableName(tn),
    m_dllName(dn),
    m_dbColumnCount(cCount)
{
}

SensorType::SensorType(QString n, QString tn, int cCount):
    id(0),
    m_name(n),
    m_dbTableName(tn),
    m_dllName(""),
    m_dbColumnCount(cCount)
{
}

/// getters
int SensorType::getId() const
{
    return id;
}
const QString &SensorType::getName() const
{
    return m_name;
}

const QString &SensorType::getDbTableName() const
{
    return m_dbTableName;
}

const QString &SensorType::getDllName() const
{
    return m_dllName;
}

/// setters
void SensorType::setId(int i)
{
    id = i;
}

void SensorType::setName(QString n)
{
    m_name = n;
}

void SensorType::setDbTableName(QString tn)
{
    m_dbTableName = tn;
}

void SensorType::setDllName(QString dn)
{
    m_dllName = dn;
}

