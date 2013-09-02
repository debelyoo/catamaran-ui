#include "timeHelper.h"

TimeHelper* TimeHelper::m_Instance = 0;

TimeHelper* TimeHelper::instance()
{
    if (!m_Instance)   // Only allow one instance of class to be generated.
    {
        m_Instance = new TimeHelper;
    }
    return m_Instance;
}

qint64 TimeHelper::labviewTsToUnixTs(double labviewTs)
{
    QDateTime labviewEpoch = QDateTime::fromString(QString("1904-01-01T00:00:00"), Qt::ISODate);
    qint64 newTs = labviewEpoch.toMSecsSinceEpoch() + math.round(labviewTs * 1000) + (3600 * 1000); // add one hour for the time zone (UTC+01:00)
    return newTs;
}
