#ifndef DATAOBJECT_H
#define DATAOBJECT_H

#include <QVector>
#include <QPair>
#include <QVariant>
#include "util/byteArrayConverter.h"

typedef QPair<QVariant, DataType::Types> DataValue;

class DataObject
{
    public:
        explicit DataObject(int address, DataValues values, qint64 ts);
        int getAddress();
        QVector<DataValue> getValues();
        qint64 getTimestamp();

    private:
        int address;
        QVector<DataValue> values;
        qint64 timestamp;

};

#endif // DATAOBJECT_H
