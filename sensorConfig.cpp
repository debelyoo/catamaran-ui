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
    if(m_sensors.contains(addr)){
        Sensor *s = m_sensors[addr];
        removeSensorFromPlots(s);
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
 * Get the sensors that are marked to be recorded (logged)
 * @brief SensorConfig::getSensorsRecorded
 * @return A list of sensors
 */
QList<Sensor*> SensorConfig::getSensorsRecorded() const
{
    QList<Sensor*> list = m_sensors.values();
    qDebug() << "# sensors: " << list.length();
    QMutableListIterator<Sensor *> i(list);
    while (i.hasNext()) {
        if (!i.next()->record())
            i.remove();
    }
    return list;
}


QString SensorConfig::getSensorsAsTabSeparatedText() const
{
    QString res = "Address\tName\tType\tDisplay\tRecord\tStream\tFilename\n";
    QList<Sensor*> list = m_sensors.values();
    qSort(list.begin(), list.end(), addressLessThan);
    foreach (Sensor* s, list) {
        res += s->address()+"\t";
        res += s->name()+"\t";
//        res += m_displayGraphs.value(s->display())+"\t";
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

bool SensorConfig::containsSensor(Sensor *s) const
{
    return m_sensors.values().contains(s);
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
/*
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
*/
bool SensorConfig::addPlot(int plotIndex)
{
    if(!m_sensorForPlotIndexMap.contains(plotIndex)){
        m_sensorForPlotIndexMap.insert(plotIndex, QList<Sensor *>());
        return true;
    }
    return false;
}

bool SensorConfig::removePlot(int plotIndex)
{
    if(m_sensorForPlotIndexMap.contains(plotIndex)){
        m_sensorForPlotIndexMap.remove(plotIndex);
        return true;
    }
    return false;
}

const QList<Sensor *> &SensorConfig::sensorForPlot(int plotIndex)
{
    return m_sensorForPlotIndexMap[plotIndex];
}

bool SensorConfig::addSensorToPlot(int plotIndex, Sensor *sensor)
{
    if(!m_sensorForPlotIndexMap[plotIndex].contains(sensor)){
        m_sensorForPlotIndexMap[plotIndex].append(sensor);
        return true;
    }
    return false;
}

bool SensorConfig::removeSensorFromPlot(int plotIndex, Sensor *sensor)
{
    return m_sensorForPlotIndexMap[plotIndex].removeOne(sensor);
}

QList<int> SensorConfig::removeSensorFromPlots(Sensor *sensor)
{
    QList<int> c;
    foreach(int i, m_sensorForPlotIndexMap.keys()){
        QList<Sensor *> sl = m_sensorForPlotIndexMap[i];
        if(sl.removeOne(sensor)){
            c.append(i);
        }
    }
    return c;
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

QDataStream &operator<<(QDataStream &stream, const SensorConfig &sc)
{
    int n=0;
    foreach(Sensor *s, sc.m_sensors.values()){
        if(s->isData()){
            ++n;
        }
    }
    stream << n;
    foreach(Sensor *s, sc.m_sensors.values()){
        if(s->isData()){
            stream << *s;
        }
    }
    stream << sc.m_sensorForPlotIndexMap.size();
    foreach(int i, sc.m_sensorForPlotIndexMap.keys()){
        QList<Sensor *> sl = sc.m_sensorForPlotIndexMap[i];
        stream << i;
        stream << sl.count();
        foreach(Sensor *s, sl){
            stream << s->address();
        }
    }
    return stream;
}


QDataStream &operator>>(QDataStream &stream, SensorConfig &sc)
{
    int ns;
    stream >> ns;
    for(int i=0;i<ns;++i){
        new Sensor(stream);
    }
    stream >> ns;
    for(int i=0;i<ns;++i){
        int n2, idx;
        stream >> idx;
        stream >> n2;
        for(int j=0;j<n2;++j){
            QString addr;
            stream >> addr;
            if(sc.m_sensors.contains(addr)){
                sc.m_sensorForPlotIndexMap[idx].append(sc.m_sensors[addr]);
            }
        }
    }
    return stream;
}
