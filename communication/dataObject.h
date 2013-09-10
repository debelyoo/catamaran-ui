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
