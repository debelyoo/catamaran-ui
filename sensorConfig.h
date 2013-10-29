#ifndef SENSORCONFIG_H
#define SENSORCONFIG_H

#include <QHash>
#include <QMap>
//#include "model/sensor.h"
#include "model/sensorType.h"

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

class Sensor;

class SensorConfig
{

    public:
        static SensorConfig* instance();

        void addSensor(Sensor* s);
        Sensor *removeSensor(const QString &addr);

        //void addSensorType(SensorType* st);

        QList<Sensor*> getSensors() const;
        QList<Sensor*> getSensorsForPlot(int plotIndex) const;
        QList<Sensor*> getSensorsRecorded() const;
        int getDisplayIndexForGraphName(QString str) const;
        QString getSensorsAsTabSeparatedText() const;
        bool containsSensor(const QString &addr) const;
        Sensor* getSensor(QString addr) const;
        //QMap<int, SensorType*> getSensorTypes() const;
        //SensorType *getSensorType(SensorList::Types type) const;
        QMap<int, QString> getDisplayValues();
        bool qstringToBool(QString str) const;
        void updateDisplayGraphList(int nb = 3);

        bool addPlot(int plotIndex);
        bool removePlot(int plotIndex);

        const QList<Sensor *> &sensorForPlot(int plotIndex);

        bool addSensorToPlot(int plotIndex, Sensor *sensor);
        bool removeSensorFromPlot(int plotIndex, Sensor *sensor);

        QList<int> removeSensorFromPlots(Sensor *sensor);

    private:
        SensorConfig():
            m_sensors(),
            m_sensorForPlotIndexMap()
        {
            //m_sensorTypes.insert(0, new SensorType(0, "Unknown", "")); // other tyes are loaded at launch from sensortypes.txt

            updateDisplayGraphList();
        }
        Q_DISABLE_COPY(SensorConfig)

        static SensorConfig* s_instance;

        QHash<QString, Sensor *> m_sensors;
        //QMap<int, SensorType*> m_sensorTypes;
        QMap<int, QString> m_displayGraphs;
        QHash<int, QList<Sensor *> > m_sensorForPlotIndexMap;

        static bool addressLessThan(Sensor* s1, Sensor* s2);
};

#endif // SENSORCONFIG_H
