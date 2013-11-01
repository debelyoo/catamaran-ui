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
        /// TODO - no more used
        static qint64 labviewTsToUnixTs(double labviewTs)
        {
            QDateTime labviewEpoch = QDateTime::fromString(QString("1904-01-01T00:00:00"), Qt::ISODate);
            qint64 newTs = labviewEpoch.toMSecsSinceEpoch() + round(labviewTs * 1000);//+(3600 * 1000)-225000; // add one hour for the time zone (UTC+01:00)
            return newTs;
        }

        /**
         * Computes the TimeZone offset (between local time and UTC)
         * For Switzerland, in summer timezone is GMT+2, and in winter GMT+1
         * @brief getTimeZoneOffset
         * @return
         */
        static QString getTimeZoneOffset()
        {
            QDateTime dt1 = QDateTime::currentDateTime();
            QDateTime dt2 = dt1.toUTC();
            dt1.setTimeSpec(Qt::UTC);

            int offset = dt2.secsTo(dt1) / 3600;

            if (offset > 0)
                return QString("+%1").arg(offset);

            return QString("-%1").arg(offset);
        }
};


#endif // TIMEHELPER_H
