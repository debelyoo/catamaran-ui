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
        void addSensor(Sensor* s);
        void addSensorType(SensorType* st);
        QList<Sensor*> getSensors();
        QList<Sensor*> getSensorsForPlot(int plotIndex);
        int getDisplayIndexForGraphName(QString str);
        QString getSensorsAsTabSeparatedText();
        bool containsSensor(int addr);
        Sensor* getSensor(int addr);
        QMap<int, SensorType*> getSensorTypes();
        QMap<int, QString> getDisplayValues();
        bool qstringToBool(QString str);
        void updateDisplayGraphList(int nb);

    private:
        SensorConfig() {
            sensorTypes.insert(0, new SensorType(0, "Unknown", "")); // other tyes are loaded at launch from sensortypes.txt

            updateDisplayGraphList(nbOfGraphs);
        }
        SensorConfig(const SensorConfig &);
        SensorConfig& operator=(const SensorConfig &);

        static SensorConfig* m_Instance;

        QHash<int, Sensor*> sensors;
        QMap<int, SensorType*> sensorTypes;
        int nbOfGraphs = 3;
        QMap<int, QString> displayGraphs;

        static bool addressLessThan(Sensor* s1, Sensor* s2);
};

#endif // SENSORCONFIG_H
