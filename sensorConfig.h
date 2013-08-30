#ifndef SENSORCONFIG_H
#define SENSORCONFIG_H

#include <QHash>
#include <QMap>
#include "sensor.h"

class SensorConfig
{

    public:
        static SensorConfig* instance();
        void addSensor(Sensor *s);
        QList<Sensor*> getSensors();
        QString getSensorsAsTabSeparatedText();
        bool containsSensor(int addr);
        Sensor* getSensor(int addr);
        QMap<int, QString> getSensorTypes();
        int NB_ADDRESSES = 75;
        bool qstringToBool(QString str);


    //public slots:

    private:
        SensorConfig() {
            sensorTypes.insert(0, "Unknown");
            sensorTypes.insert(1, "GPS position");
            sensorTypes.insert(2, "GPS speed");
            sensorTypes.insert(3, "PT100");
            sensorTypes.insert(4, "Wind speed");
            sensorTypes.insert(5, "Wind direction");
            sensorTypes.insert(6, "Radiometer");
            /*sensorTypes << "Unknown" << "GPS position" << "GPS speed" << "PT100"
                        << "Wind speed" << "Wind direction" << "Radiometer";
                        */
        }
        SensorConfig(const SensorConfig &);
        SensorConfig& operator=(const SensorConfig &);

        static SensorConfig* m_Instance;

        QHash<int, Sensor*> sensors;
        //QList<QString> sensorTypes;
        QMap<int, QString> sensorTypes;

        static bool addressLessThan(Sensor* s1, Sensor* s2);
};

#endif // SENSORCONFIG_H
