#ifndef TIMEHELPER_H
#define TIMEHELPER_H

#include <QDateTime>
#include <math.h>

/**
 * @brief The TimeHelper class
 */
class TimeHelper
{

    public:
        static qint64 labviewTsToUnixTs(double labviewTs) {
            QDateTime labviewEpoch = QDateTime::fromString(QString("1904-01-01T00:00:00"), Qt::ISODate);
            qint64 newTs = labviewEpoch.toMSecsSinceEpoch() + round(labviewTs * 1000) + (3600 * 1000); // add one hour for the time zone (UTC+01:00)
            return newTs;
        }
};


#endif // TIMEHELPER_H
