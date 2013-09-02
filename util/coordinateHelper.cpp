#include "coordinateHelper.h";

CoordinateHelper* CoordinateHelper::m_Instance = 0;

CoordinateHelper* CoordinateHelper::instance()
{
    if (!m_Instance)   // Only allow one instance of class to be generated.
    {
        m_Instance = new CoordinateHelper;
    }
    return m_Instance;
}

/**
 * Convert LV03 to WGS84 Return a array of double that contain lat, long, and height
 * taken from http://www.swisstopo.admin.ch/internet/swisstopo/en/home/products/software/products/skripts.html
 *
 * @param east
 * @param north
 * @param height
 * @return
 */
QVector<double> CoordinateHelper::LV03toWGS84(double east, double north, double height)
{

    //double d[] = new double[3];
    QVector<double> d(3);

    d[0] = CHtoWGSlat(east, north);
    d[1] = CHtoWGSlng(east, north);
    d[2] = CHtoWGSheight(east, north, height);
    return d;
}

/**
 * Convert WGS84 to LV03 Return an array of double that contaign east, north, and height
 * taken from http://www.swisstopo.admin.ch/internet/swisstopo/en/home/products/software/products/skripts.html
 *
 * @param latitude
 * @param longitude
 * @param ellHeight
 * @return
 */
QVector<double> CoordinateHelper::WGS84toLV03(double latitude, double longitude, double ellHeight)
{
    // , ref double east, ref double north, ref double height
    //double d[] = new double[3];
    QVector<double> d(3);

    d[0] = WGStoCHy(latitude, longitude);
    d[1] = WGStoCHx(latitude, longitude);
    d[2] = WGStoCHh(latitude, longitude, ellHeight);
    return d;
}

// Convert decimal angle (degrees) to sexagesimal angle (degrees, minutes
// and seconds dd.mmss,ss)
double CoordinateHelper::DecToSexAngle(double dec)
{
    int deg = (int) floor(dec);
    int min = (int) floor((dec - deg) * 60);
    double sec = (((dec - deg) * 60) - min) * 60;

    // Output: dd.mmss(,)ss
    return deg + ((double) min / 100) + (sec / 10000);
}

// Convert sexagesimal angle (degrees, minutes and seconds dd.mmss,ss) to
// seconds
double CoordinateHelper::SexAngleToSeconds(double dms)
{
    double deg = 0, min = 0, sec = 0;
    deg = floor(dms);
    min = floor((dms - deg) * 100);
    sec = (((dms - deg) * 100) - min) * 100;

    // Result in degrees sex (dd.mmss)
    return sec + (min * 60) + (deg * 3600);
}

// Convert sexagesimal angle (degrees, minutes and seconds "dd.mmss") to
// decimal angle (degrees)
double CoordinateHelper::SexToDecAngle(double dms) {
    // Extract DMS
    // Input: dd.mmss(,)ss
    double deg = 0, min = 0, sec = 0;
    deg = floor(dms);
    min = floor((dms - deg) * 100);
    sec = (((dms - deg) * 100) - min) * 100;

    // Result in degrees dec (dd.dddd)
    return deg + (min / 60) + (sec / 3600);
}

/**
 * Convert CH coordinates (east, north) to position on UI map (x, y)
 * @brief LV03toUIMap
 * @param chx
 * @param chy
 * @return a vector with x and y coordinates
 */
QVector<double> CoordinateHelper::LV03toUIMap(int chx, int chy)
{
    QVector<double> d(2);
    int xMax = 564446;
    int xMin = 497605;
    int yMax = 155320;
    int yMin = 116755;
    int imgWidth = 11349;
    int imgHeight = 6548;
    double xMap, yMap = 0.0;
    if (chx >= xMin && chx <= xMax && chy >= yMin && chy <= yMax)
    {
        xMap = ((double)(chx - xMin) / (xMax - xMin)) * imgWidth;
        yMap = imgHeight - (((double)(chy - yMin) / (yMax - yMin)) * imgHeight);
    }
    d[0] = xMap;
    d[1] = yMap;
    return d;
}

/// Private functions

// Convert CH y/x/h to WGS height
double CoordinateHelper::CHtoWGSheight(double y, double x, double h)
{
    // Converts militar to civil and to unit = 1000km
    // Axiliary values (% Bern)
    double y_aux = (y - 600000) / 1000000;
    double x_aux = (x - 200000) / 1000000;

    // Process height
    h = (h + 49.55) - (12.60 * y_aux) - (22.64 * x_aux);

    return h;
}

// Convert CH y/x to WGS lat
double CoordinateHelper::CHtoWGSlat(double y, double x)
{
    // Converts militar to civil and to unit = 1000km
    // Axiliary values (% Bern)
    double y_aux = (y - 600000) / 1000000;
    double x_aux = (x - 200000) / 1000000;

    // Process lat
    double lat = (16.9023892 + (3.238272 * x_aux))
            - (0.270978 * pow(y_aux, 2))
            - (0.002528 * pow(x_aux, 2))
            - (0.0447 * pow(y_aux, 2) * x_aux)
            - (0.0140 * pow(x_aux, 3));

    // Unit 10000" to 1 " and converts seconds to degrees (dec)
    lat = (lat * 100) / 36;

    return lat;
}

// Convert CH y/x to WGS long
double CoordinateHelper::CHtoWGSlng(double y, double x)
{
    // Converts militar to civil and to unit = 1000km
    // Axiliary values (% Bern)
    double y_aux = (y - 600000) / 1000000;
    double x_aux = (x - 200000) / 1000000;

    // Process long
    double lng = (2.6779094 + (4.728982 * y_aux)
            + (0.791484 * y_aux * x_aux) + (0.1306 * y_aux * pow(
            x_aux, 2))) - (0.0436 * pow(y_aux, 3));

    // Unit 10000" to 1 " and converts seconds to degrees (dec)
    lng = (lng * 100) / 36;

    return lng;
}

// Convert WGS lat/long (° dec) and height to CH h
double CoordinateHelper::WGStoCHh(double lat, double lng, double h)
{
    // Converts degrees dec to sex
    lat = DecToSexAngle(lat);
    lng = DecToSexAngle(lng);

    // Converts degrees to seconds (sex)
    lat = SexAngleToSeconds(lat);
    lng = SexAngleToSeconds(lng);

    // Axiliary values (% Bern)
    double lat_aux = (lat - 169028.66) / 10000;
    double lng_aux = (lng - 26782.5) / 10000;

    // Process h
    h = (h - 49.55) + (2.73 * lng_aux) + (6.94 * lat_aux);

    return h;
}

// Convert WGS lat/long (° dec) to CH x
double CoordinateHelper::WGStoCHx(double lat, double lng) {
    // Converts degrees dec to sex
    lat = DecToSexAngle(lat);
    lng = DecToSexAngle(lng);

    // Converts degrees to seconds (sex)
    lat = SexAngleToSeconds(lat);
    lng = SexAngleToSeconds(lng);

    // Axiliary values (% Bern)
    double lat_aux = (lat - 169028.66) / 10000;
    double lng_aux = (lng - 26782.5) / 10000;

    // Process X
    double x = ((200147.07 + (308807.95 * lat_aux)
            + (3745.25 * pow(lng_aux, 2)) + (76.63 * pow(lat_aux,
            2))) - (194.56 * pow(lng_aux, 2) * lat_aux))
            + (119.79 * pow(lat_aux, 3));

    return x;
}

// Convert WGS lat/long (° dec) to CH y
double CoordinateHelper::WGStoCHy(double lat, double lng) {
    // Converts degrees dec to sex
    lat = DecToSexAngle(lat);
    lng = DecToSexAngle(lng);

    // Converts degrees to seconds (sex)
    lat = SexAngleToSeconds(lat);
    lng = SexAngleToSeconds(lng);

    // Axiliary values (% Bern)
    double lat_aux = (lat - 169028.66) / 10000;
    double lng_aux = (lng - 26782.5) / 10000;

    // Process Y
    double y = (600072.37 + (211455.93 * lng_aux))
            - (10938.51 * lng_aux * lat_aux)
            - (0.36 * lng_aux * pow(lat_aux, 2))
            - (44.54 * pow(lng_aux, 3));

    return y;
}

