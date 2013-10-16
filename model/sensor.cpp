#include "sensor.h"

Sensor::Sensor(QString addr, QString name, const SensorType* type,
               int display, bool record, bool stream, QString logFilePrefix, QString currentLogFilename):
    m_address(addr),
    m_name(name),
    m_type(type),
    m_record(record),
    m_stream(stream),
    m_display(display),
    m_logFilePrefix(logFilePrefix),
    m_currentLogFilename(currentLogFilename)
{
    SensorConfig::instance()->addSensor(this);
}

//Sensor::Sensor(Sensor *parent, QString addr, QString name, SensorType *type, int display, bool record, bool stream, QString logFilePrefix, QString currentLogFilename):
//    m_parent(parent),
//    m_address(addr),
//    m_name(name),
//    m_type(type),
//    m_display(display),
//    m_record(record),
//    m_stream(stream),
//    m_logFilePrefix(logFilePrefix),
//    m_currentLogFilename(currentLogFilename)
//{
//    if(m_parent){
//        m_parent->addChild(this);
//    }
//}

Sensor::~Sensor()
{
    SensorConfig::instance()->removeSensor(m_address);
}

int Sensor::display() const
{
    return m_display;
}

const QString &Sensor::address() const
{
    return m_address;
}

const QString &Sensor::name() const
{
    return m_name;
}

const SensorType *Sensor::type() const
{
    return m_type;
}

bool Sensor::stream() const
{
    return m_stream;
}

bool Sensor::record() const
{
    return m_record;
}

const TransformationBaseClass *Sensor::transformation() const
{
    return m_transformation;
}

const QString &Sensor::logFilePrefix() const
{
    return m_logFilePrefix;
}

const QString &Sensor::currentLogFilename() const
{
    return m_currentLogFilename;
}

void Sensor::setName(const QString &name)
{
    m_name = name;
}

void Sensor::setDisplay(int g)
{
    m_display = g;
}

void Sensor::setType(const SensorType *type)
{
    m_type = type;
}

void Sensor::setStream(bool enable)
{
    m_stream = enable;
}

void Sensor::setRecord(bool enable)
{
    m_record = enable;
}

void Sensor::setTransformation(const TransformationBaseClass *tr)
{
    m_transformation = tr;
}

void Sensor::setLogFilePrefix(const QString &prefix)
{
    m_logFilePrefix = prefix;
}

void Sensor::setCurrentLogFilename(const QString &fn)
{
    m_currentLogFilename = fn;
}

/*
Sensor::Sensor(int addr, QString n, SensorType* t,
               int d, bool rec, bool st, QString logPrefix, QString lfn)
{
    m_address(addr),
    m_name(n),
    m_type(t),
    m_display(d),
    m_record(rec),
    m_stream(st),
    m_logFilePrefix(logPrefix),
    m_currentLogFilename(lfn),
}

/// getters
int Sensor::getAddress()
{
    return address;
}

QString Sensor::getName()
{
    return name;
}

SensorType* Sensor::getType()
{
    return type;
}

int Sensor::getDisplay()
{
    return display;
}

bool Sensor::getRecord()
{
    return record;
}

bool Sensor::getStream()
{
    return stream;
}

QString Sensor::getLogFilePrefix()
{
    return logFilePrefix;
}

QString Sensor::getCurrentLogFilename()
{
    return currentLogFilename;
}

/// setters
void Sensor::setAddress(int addr)
{
    m_address(addr),
}

void Sensor::setName(QString n)
{
    m_name(n),
}

void Sensor::setType(SensorType* t)
{
    m_type(t),
}

void Sensor::setDisplay(int d)
{
    m_display(d),
}

void Sensor::setRecord(bool b)
{
    m_record(b),
}

void Sensor::setStream(bool b)
{
    m_stream(b),
}

void Sensor::setLogFilePrefix(QString prefix){
    m_logFilePrefix(prefix),
}

void Sensor::setCurrentLogFilename(QString fn)
{
    m_currentLogFilename(fn),
}
*/
