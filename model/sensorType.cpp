#include "sensorType.h"

SensorType::SensorType(int i, QString n, QString tn, QString dn)
{
    id = i;
    name = n;
    dbTableName = tn;
    dllName = dn;
}

/// getters
int SensorType::getId()
{
    return id;
}
QString SensorType::getName()
{
    return name;
}

QString SensorType::getDbTableName()
{
    return dbTableName;
}

QString SensorType::getDllName()
{
    return dllName;
}

/// setters
void SensorType::setId(int i)
{
    id = i;
}

void SensorType::setName(QString n)
{
    name = n;
}

void SensorType::setDbTableName(QString tn)
{
    dbTableName = tn;
}

void SensorType::setDllName(QString dn)
{
    dllName = dn;
}

