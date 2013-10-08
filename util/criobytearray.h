#ifndef CRIOBYTEARRAY_H
#define CRIOBYTEARRAY_H

#include <QByteArray>
#include <QVariant>
#include <QPointF>
#include <QDataStream>
#include "util/criodefinitions.h"
#include "communication/criodata.h"
#include "communication/criocommand.h"
#include "model/sensor.h"

class CRioCommand;
class CRioData;

class CRioByteArray
{
public:
    CRioByteArray();
    CRioByteArray(const CRioCommand &cmd);
    CRioByteArray(const CRioData &data);

    int size() const;

    QByteArray &operator <<(QByteArray &ba);

    const QByteArray &byteArray() const;

    void push(const CRioCommand &cmd);
    void push(const CRioData &data);
protected:
    QByteArray m_byteArray;
private:
    void push(const QByteArray &a, bool flatten = false);
    void push(const qint8 &a, bool flatten = false);
    void push(const quint8 &a, bool flatten = false);
    void push(const qint16 &a, bool flatten = false);
    void push(const quint16 &a, bool flatten = false);
    void push(const qint32 &a, bool flatten = false);
    void push(const quint32 &a, bool flatten = false);
    void push(const qint64 &a, bool flatten = false);
    void push(const quint64 &a, bool flatten = false);
    void push(const double &a, bool flatten = false);
    void push(const float &a, bool flatten = false);
    void push(const bool &a, bool flatten = false);

    void push(const CRioByteArray &a, bool flatten = false);
    void push(const QPointF &a, bool flatten = false);
    void push(const QVariant &v, bool flatten = false);
    void push(const CRIO::PolymorphicData &v, bool flatten = false);
    void push(const CRIO::Timestamp &ts, bool flatten = false);
    void push(const QList<Sensor *> sensors, bool flatten = false);
};

inline QByteArray &operator <<(QByteArray &ba, const CRioByteArray &cba);
inline QDataStream &operator <<(QDataStream &ds, const CRioByteArray &cba);

#endif // CRIOBYTEARRAY_H
