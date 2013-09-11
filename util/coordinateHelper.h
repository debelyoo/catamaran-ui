#ifndef COORDINATEHELPER_H
#define COORDINATEHELPER_H

#include <QVector>
#include <math.h>
#include <QPointF>

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
        QPointF LV03toUIMap(int chx, int chy);
        QPointF UIMaptoLV03(QPointF p);

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

        // Map values (jpg file)
        const static int xMax = 564446;
        const static int xMin = 497605;
        const static int yMax = 155320;
        const static int yMin = 116755;
        const static int imgWidth = 11349;
        const static int imgHeight = 6548;
};

#endif // COORDINATEHELPER_H
