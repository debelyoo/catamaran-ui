#include "sensor.h"

Sensor::Sensor(int addr, QString n, SensorType* t,
               int d, bool rec, bool st, QString logPrefix, QString lfn)
{
    address = addr;
    name = n;
    type = t;
    display = d;
    record = rec;
    stream = st;
    logFilePrefix = logPrefix;
    currentLogFilename = lfn;
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
    address = addr;
}

void Sensor::setName(QString n)
{
    name = n;
}

void Sensor::setType(SensorType* t)
{
    type = t;
}

void Sensor::setDisplay(int d)
{
    display = d;
}

void Sensor::setRecord(bool b)
{
    record = b;
}

void Sensor::setStream(bool b)
{
    stream = b;
}

void Sensor::setLogFilePrefix(QString prefix){
    logFilePrefix = prefix;
}

void Sensor::setCurrentLogFilename(QString fn)
{
    currentLogFilename = fn;
}
