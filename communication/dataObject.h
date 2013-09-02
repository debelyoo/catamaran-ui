#ifndef DATAOBJECT_H
#define DATAOBJECT_H

#include <QVector>
#include <QPair>
#include <QVariant>
#include "util/byteArrayConverter.h"

class DataObject
{
    public:
        explicit DataObject(int address, QVector< QPair<QVariant, DataType::Types> > values, qint64 ts);
        int getAddress();
        QVector< QPair<QVariant, DataType::Types> > getValues();
        qint64 getTimestamp();

    private:
        int address;
        QVector< QPair<QVariant, DataType::Types> > values;
        qint64 timestamp;

};

#endif // DATAOBJECT_H
