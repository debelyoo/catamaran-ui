#include "dataObject.h"

DataObject::DataObject(int addr, QVector<DataValue> vals, qint64 ts)
{
    address = addr;
    values = vals;
    timestamp = ts;
}

int DataObject::getAddress() const
{
    return address;
}

QVector<DataValue> DataObject::getValues() const
{
    return values;
}

qint64 DataObject::getTimestamp() const
{
    return timestamp;
}
