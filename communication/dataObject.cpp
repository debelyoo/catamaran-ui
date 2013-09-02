#include "dataObject.h"

DataObject::DataObject(int addr, QVector< QPair<QVariant, DataType::Types> > vals, qint64 ts)
{
    address = addr;
    values = vals;
    timestamp = ts;
}

int DataObject::getAddress()
{
    return address;
}

QVector< QPair<QVariant, DataType::Types> > DataObject::getValues()
{
    return values;
}

qint64 DataObject::getTimestamp()
{
    return timestamp;
}
