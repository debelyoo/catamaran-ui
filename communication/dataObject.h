#ifndef DATAOBJECT_H
#define DATAOBJECT_H

#include <QVector>
#include <QPair>
#include <QVariant>
#ifdef DLL_BUILD
#include "../util/definitions.h"
#else
#include "util/definitions.h"
#endif

typedef QPair<QVariant, DataType::Types> DataValue;

class DataObject
{
    public:
        explicit DataObject(int address, QVector<DataValue> values, qint64 ts);
        int getAddress() const;
        QVector<DataValue> getValues() const;
        qint64 getTimestamp() const;
    private:
        int address;
        QVector<DataValue> values;
        qint64 timestamp;

};

#endif // DATAOBJECT_H
