#ifndef COORDINATEHELPER_H
#define COORDINATEHELPER_H

#include <QVector>
#include <math.h>

/**
 * @brief The CoordinateHelper class - Singleton
 */
class CoordinateHelper
{

    public:
        static CoordinateHelper* instance();

        QVector<double> LV03toWGS84(double east, double north, double height);
        QVector<double> WGS84toLV03(double latitude, double longitude, double ellHeight);
        double DecToSexAngle(double dec);
        double SexAngleToSeconds(double dms);
        double SexToDecAngle(double dms);
        QVector<double> LV03toUIMap(int chx, int chy);

    private:
        CoordinateHelper() {}
        CoordinateHelper(const CoordinateHelper &);
        CoordinateHelper& operator=(const CoordinateHelper &);

        static CoordinateHelper* m_Instance;

        double CHtoWGSheight(double y, double x, double h);
        double CHtoWGSlat(double y, double x);
        double CHtoWGSlng(double y, double x);
        double WGStoCHh(double lat, double lng, double h);
        double WGStoCHx(double lat, double lng);
        double WGStoCHy(double lat, double lng);
};

#endif // COORDINATEHELPER_H
