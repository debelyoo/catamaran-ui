#ifndef SENSORCONFIG_H
#define SENSORCONFIG_H

#include <QHash>
#include <QMap>
#include "model/sensor.h"
#include "model/sensorType.h"

class SensorConfig
{

    public:
        static SensorConfig* instance();
        void addSensor(Sensor *s);
        QList<Sensor*> getSensors();
        QList<Sensor*> getSensorsForPlot(int plotIndex);
        int getDisplayIndexForGraphName(QString str);
        QString getSensorsAsTabSeparatedText();
        bool containsSensor(int addr);
        Sensor* getSensor(int addr);
        QMap<int, SensorType*> getSensorTypes();
        QMap<int, QString> getDisplayValues();
        bool qstringToBool(QString str);


    //public slots:

    private:
        SensorConfig() {
            sensorTypes.insert(0, new SensorType(0, "Unknown", ""));
            sensorTypes.insert(1, new SensorType(1, "GPS position", "gpslog"));
            sensorTypes.insert(2, new SensorType(2, "GPS speed", "gpslog"));
            sensorTypes.insert(3, new SensorType(3, "PT100", "temperaturelog"));
            sensorTypes.insert(4, new SensorType(4, "Wind speed", "windlog"));
            sensorTypes.insert(5, new SensorType(5, "Wind direction", "windlog"));
            sensorTypes.insert(6, new SensorType(6, "Radiometer", "radiometerlog"));

            displayGraphs.insert(0, "NO");
            displayGraphs.insert(1, "G0");
            displayGraphs.insert(2, "G1");
            displayGraphs.insert(3, "G2");
        }
        SensorConfig(const SensorConfig &);
        SensorConfig& operator=(const SensorConfig &);

        static SensorConfig* m_Instance;

        QHash<int, Sensor*> sensors;
        QMap<int, SensorType*> sensorTypes;
        QMap<int, QString> displayGraphs;

        static bool addressLessThan(Sensor* s1, Sensor* s2);
};

#endif // SENSORCONFIG_H
