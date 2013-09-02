#ifndef SENSOR_H
#define SENSOR_H

#include <QObject>

namespace SensorType {
    typedef enum {
        Unknown = 0,
        GPS_position = 1,
        GPS_speed = 2,
        PT100 = 3,
        Wind_speed = 4,
        Wind_direction = 5,
        Radiometer = 6
    } Types;
}

class Sensor
{
    public:
    explicit Sensor(int addr = 0, QString name = "",
                    int type = 0, QString display = "NO",
                    bool record = false, bool stream = false, QString filename = "");
    /// getters
    int getAddress();
    QString getName();
    int getType();
    QString getDisplay();
    bool getRecord();
    bool getStream();
    QString getFilename();

    /// setters
    void setAddress(int addr);
    void setName(QString name);
    void setType(int);
    void setDisplay(QString display);
    void setRecord(bool b);
    void setStream(bool b);
    void setFilename(QString fn);

    private:
        int address;
        QString name;
        int type;
        QString display;
        bool record;
        bool stream;
        QString filename;
};

#endif // SENSOR_H
