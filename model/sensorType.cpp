#include "sensorType.h"

SensorType::SensorType(const QString &n):
    m_name(n)
{
}



/// getters
const QString &SensorType::name() const
{
    return m_name;
}

/// setters
void SensorType::setName(QString n)
{
    m_name = n;
}


QDataStream &operator<<(QDataStream &stream, const SensorType &sensorType)
{
    stream<<sensorType.name();
}
