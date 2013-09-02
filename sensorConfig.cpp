#include "sensorConfig.h"
#include <QtAlgorithms>

SensorConfig* SensorConfig::m_Instance = 0;

/** This function is called to create an instance of the class.
    Calling the constructor publicly is not allowed. The constructor
    is private and is only called by this Instance function.
*/
SensorConfig* SensorConfig::instance()
{
    if (!m_Instance)   // Only allow one instance of class to be generated.
    {
        m_Instance = new SensorConfig;
    }
    return m_Instance;
}

void SensorConfig::addSensor(Sensor *s)
{
    sensors.insert(s->getAddress(), s);
}

QList<Sensor*> SensorConfig::getSensors()
{
    QList<Sensor*> list = sensors.values();
    //std::sort(list.begin(), list.end(), addressLessThan);
    qSort(list.begin(), list.end(), addressLessThan);
    return list;
}

QString SensorConfig::getSensorsAsTabSeparatedText()
{
    QString res = "Address\tName\tType\tDisplay\tRecord\tStream\tFilename\t\n";
    QList<Sensor*> list = sensors.values();
    qSort(list.begin(), list.end(), addressLessThan);
    foreach (Sensor* s, list) {
        res += QString::number(s->getAddress())+"\t";
        res += s->getName()+"\t";
        res += QString::number(s->getType())+"\t";
        res += s->getDisplay()+"\t";
        res += QString::number(s->getRecord())+"\t";
        res += QString::number(s->getStream())+"\t";
        res += s->getFilename()+"\t\n";
    }
    return res;
}

bool SensorConfig::containsSensor(int addr)
{
    return sensors.contains(addr);
}

Sensor* SensorConfig::getSensor(int addr)
{
    return sensors[addr];
}

QMap<int, QString> SensorConfig::getSensorTypes()
{
    return sensorTypes;
}

bool SensorConfig::qstringToBool(QString str)
{
    bool b;
    if (str == "YES") {
        b = true;
    } else {
        b = false;
    }
    return b;
}

/**
 * Compare sensors by address
 * @brief SensorConfig::addressLessThan
 * @param s1 The first sensor
 * @param s2 The second sensor
 * @return true if sensor1's address is smaller than sensor2's one, false otherwise
 */
bool SensorConfig::addressLessThan(Sensor* s1, Sensor* s2)
{
     return s1->getAddress() < s2->getAddress();
}


