#include "sensor.h"

Sensor::Sensor(int addr, QString n, int t,
               QString d, bool rec, bool st, QString fn)
{
    address = addr;
    name = n;
    type = t;
    display = d;
    record = rec;
    stream = st;
    filename = fn;
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

int Sensor::getType()
{
    return type;
}

QString Sensor::getDisplay()
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

QString Sensor::getFilename()
{
    return filename;
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

void Sensor::setType(int t)
{
    type = t;
}

void Sensor::setDisplay(QString d)
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

void Sensor::setFilename(QString fn)
{
    filename = fn;
}
