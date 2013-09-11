#ifndef BYTEARRAYCONVERTER_H
#define BYTEARRAYCONVERTER_H

#include "model/sensor.h"
#include <QObject>
#include <QDataStream>
#include <QVector>
#include <QPoint>

/**
 * @brief The ByteArrayConverter class - Singleton
 */
class ByteArrayConverter
{

    public:
        static ByteArrayConverter* instance();

        quint8 byteArrayToUInt8(QByteArray ba);
        quint16 byteArrayToUInt16(QByteArray ba);
        quint32 byteArrayToUInt32(QByteArray ba);
        quint64 byteArrayToUInt64(QByteArray ba);
        qint8 byteArrayToInt8(QByteArray ba);
        qint16 byteArrayToInt16(QByteArray ba);
        qint32 byteArrayToInt32(QByteArray ba);
        qint64 byteArrayToInt64(QByteArray ba);
        double byteArrayToDouble(QByteArray ba);
        QByteArray intToByteArray(int val, int byteArraySize);
        QByteArray getFirstBytesOfArray(QByteArray ba, int n);
        QByteArray getLastBytesOfArray(QByteArray ba, int n);
        QByteArray invertBytes(QByteArray ba);
        QByteArray byteArrayForCmdParameterInt(int val); // return a byte array with length and value
        QByteArray byteArrayForCmdParameterClusterOfPoints(QList<QPointF> pts);
        QByteArray byteArrayForCmdParameterStreamArray(QList<Sensor*> sensors); // return a byte array with length and values (bool, address)

    public slots:

    private:
        ByteArrayConverter() {}
        ByteArrayConverter(const ByteArrayConverter &);
        ByteArrayConverter& operator=(const ByteArrayConverter &);

        static ByteArrayConverter* m_Instance;
};

#endif // BYTEARRAYCONVERTER_H
