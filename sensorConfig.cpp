#include "sensorConfig.h"
#include "model/sensor.h"
#include <QtAlgorithms>

SensorConfig* SensorConfig::s_instance = 0;

/** This function is called to create an instance of the class.
    Calling the constructor publicly is not allowed. The constructor
    is private and is only called by this Instance function.
*/
SensorConfig* SensorConfig::instance()
{
    if (!s_instance)   // Only allow one instance of class to be generated.
    {
        s_instance = new SensorConfig;
    }
    return s_instance;
}

void SensorConfig::addSensor(Sensor* s)
{
    m_sensors.insert(s->address(), s);
}

Sensor *SensorConfig::removeSensor(const QString &addr)
{
    if(Sensor *s = m_sensors[addr]){
        m_sensors.remove(addr);
        //delete s;
        return s;
    }
    return NULL;
}

//void SensorConfig::addSensorType(SensorType* st)
//{
//    m_sensorTypes.insert(st->getId(), st);
//}

/**
 * Get the sensors as a QList
 * @brief SensorConfig::getSensors
 * @return
 */
QList<Sensor*> SensorConfig::getSensors() const
{
    QList<Sensor*> list = m_sensors.values();
    qSort(list.begin(), list.end(), addressLessThan);
    return list;
}

/**
 * Get the sensors to be plotted on plot with index "plotIndex"
 * @brief SensorConfig::getSensorsForPlot
 * @param plotIndex The index of the desired plot
 * @return A list of sensors
 */
QList<Sensor*> SensorConfig::getSensorsForPlot(int plotIndex) const
{
    QList<Sensor*> list = m_sensors.values();
    QMutableListIterator<Sensor *> i(list);
    while (i.hasNext()) {
        if (i.next()->display() != plotIndex)
            i.remove();
    }
    return list;
}

int SensorConfig::getDisplayIndexForGraphName(QString gName) const
{
    int gIndex;
    QMap<int, QString>::ConstIterator i;
    for (i = m_displayGraphs.constBegin(); i != m_displayGraphs.constEnd(); ++i)
    {
        if (i.value() == gName)
            gIndex = i.key();
    }
    return gIndex;
}

QString SensorConfig::getSensorsAsTabSeparatedText() const
{
    QString res = "Address\tName\tType\tDisplay\tRecord\tStream\tFilename\n";
    QList<Sensor*> list = m_sensors.values();
    qSort(list.begin(), list.end(), addressLessThan);
    foreach (Sensor* s, list) {
        res += s->address()+"\t";
        res += s->name()+"\t";
        res += QString::number(s->type()->getId())+"\t";
        res += m_displayGraphs.value(s->display())+"\t";
        res += QString::number(s->record())+"\t";
        res += QString::number(s->stream())+"\t";
        res += s->logFilePrefix()+"\n";
    }
    return res;
}

bool SensorConfig::containsSensor(const QString &addr) const
{
    return m_sensors.contains(addr);
}

Sensor* SensorConfig::getSensor(QString addr) const
{
    if(!m_sensors.contains(addr)){
        return NULL;
    }
    return m_sensors[addr];
}

//QMap<int, SensorType*> SensorConfig::getSensorTypes() const
//{
//    return m_sensorTypes;
//}

//SensorType *SensorConfig::getSensorType(SensorList::Types type) const
//{
//    if(m_sensorTypes.contains((int) type)){
//        return m_sensorTypes[(int) type];
//    }
//    return NULL;
//}

QMap<int, QString> SensorConfig::getDisplayValues()
{
    return m_displayGraphs;
}

bool SensorConfig::qstringToBool(QString str) const
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
    m_displayGraphs.clear();
    m_displayGraphs.insert(0, "NO");
    for (int i = 0; i < nb; i++)
    {
        m_displayGraphs.insert(i+1, "G"+QString::number(i));
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
    QString addr1 = s1->address();
    QString addr2 = s2->address();
    int ms = std::min(addr1.length(), addr1.length());
    for(int i=0; i < ms; ++i){
        if(addr1[i] != addr2[i]){
            return addr1[i] < addr2[i];
        }
    }
    return false;
}


