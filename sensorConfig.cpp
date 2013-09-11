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

void SensorConfig::addSensor(Sensor* s)
{
    sensors.insert(s->getAddress(), s);
}

void SensorConfig::addSensorType(SensorType* st)
{
    sensorTypes.insert(st->getId(), st);
}

/**
 * Get the sensors as a QList
 * @brief SensorConfig::getSensors
 * @return
 */
QList<Sensor*> SensorConfig::getSensors()
{
    QList<Sensor*> list = sensors.values();
    qSort(list.begin(), list.end(), addressLessThan);
    return list;
}

/**
 * Get the sensors to be plotted on plot with index "plotIndex"
 * @brief SensorConfig::getSensorsForPlot
 * @param plotIndex The index of the desired plot
 * @return A list of sensors
 */
QList<Sensor*> SensorConfig::getSensorsForPlot(int plotIndex)
{
    QList<Sensor*> list = sensors.values();
    QMutableListIterator<Sensor *> i(list);
    while (i.hasNext()) {
        if (i.next()->getDisplay() != plotIndex)
            i.remove();
    }
    return list;
}

int SensorConfig::getDisplayIndexForGraphName(QString gName)
{
    int gIndex;
    QMap<int, QString>::iterator i;
    for (i = displayGraphs.begin(); i != displayGraphs.end(); ++i)
    {
        if (i.value() == gName)
            gIndex = i.key();
    }
    return gIndex;
}

QString SensorConfig::getSensorsAsTabSeparatedText()
{
    QString res = "Address\tName\tType\tDisplay\tRecord\tStream\tFilename\n";
    QList<Sensor*> list = sensors.values();
    qSort(list.begin(), list.end(), addressLessThan);
    foreach (Sensor* s, list) {
        res += QString::number(s->getAddress())+"\t";
        res += s->getName()+"\t";
        res += QString::number(s->getType()->getId())+"\t";
        res += displayGraphs.value(s->getDisplay())+"\t";
        res += QString::number(s->getRecord())+"\t";
        res += QString::number(s->getStream())+"\t";
        res += s->getLogFilePrefix()+"\n";
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

QMap<int, SensorType*> SensorConfig::getSensorTypes()
{
    return sensorTypes;
}

QMap<int, QString> SensorConfig::getDisplayValues()
{
    return displayGraphs;
}

bool SensorConfig::qstringToBool(QString str)
{
    bool b;
    if (str == "1") {
        b = true;
    } else {
        b = false;
    }
    return b;
}

void SensorConfig::updateDisplayGraphList(int nb)
{
    displayGraphs.clear();
    displayGraphs.insert(0, "NO");
    for (int i = 0; i < nb; i++)
    {
        displayGraphs.insert(i+1, "G"+QString::number(i));
    }
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


