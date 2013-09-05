#ifndef SENSOR_H
#define SENSOR_H

#include <QObject>
#include "sensorType.h"

namespace SensorList {
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
                    SensorType* type = 0, int display = 0,
                    bool record = false, bool stream = false,
                    QString logFilePrefix = "", QString currentLogFilename = "");
    /// getters
    int getAddress();
    QString getName();
    SensorType* getType();
    int getDisplay();
    bool getRecord();
    bool getStream();
    QString getLogFilePrefix();
    QString getCurrentLogFilename();

    /// setters
    void setAddress(int addr);
    void setName(QString name);
    void setType(SensorType* t);
    void setDisplay(int display);
    void setRecord(bool b);
    void setStream(bool b);
    void setLogFilePrefix(QString prefix);
    void setCurrentLogFilename(QString fn);

    private:
        int address;
        QString name;
        SensorType* type;
        int display;
        bool record;
        bool stream;
        QString logFilePrefix;
        QString currentLogFilename;
};

#endif // SENSOR_H
