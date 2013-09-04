#include "sensorType.h"

SensorType::SensorType(int i, QString n, QString tn)
{
    id = i;
    name = n;
    dbTableName = tn;
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

