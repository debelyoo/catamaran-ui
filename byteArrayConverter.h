#ifndef BYTEARRAYCONVERTER_H
#define BYTEARRAYCONVERTER_H

#include "sensor.h"
#include <QObject>
#include <QDataStream>
#include <QVector>

namespace DataType {
    typedef enum {
        Double = 0,
        QString = 1,
        FXP2410 = 2,
        Int32 = 3,
        Int16 = 4,
        Int8 = 5,
        UInt32 = 6,
        UInt16 = 7,
        UInt8 = 8,
        LightAddrConf = 9,
        SBG_IG_500E_Output = 10
    } Types;
}

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
        qint64 byteArrayToInt64(QByteArray ba);
        double byteArrayToDouble(QByteArray ba);
        QByteArray intToByteArray(int val, int byteArraySize);
        QByteArray getFirstBytesOfArray(QByteArray ba, int n);
        QByteArray getLastBytesOfArray(QByteArray ba, int n);
        QByteArray invertBytes(QByteArray ba);
        QByteArray byteArrayForCmdParameterInt(int val); // return a byte array with length and value
        QByteArray byteArrayForCmdParameterStreamArray(QList<Sensor*> sensors); // return a byte array with length and values (bool, address)

    public slots:

    private:
        ByteArrayConverter() {}
        ByteArrayConverter(const ByteArrayConverter &);
        ByteArrayConverter& operator=(const ByteArrayConverter &);

        static ByteArrayConverter* m_Instance;
};

#endif // BYTEARRAYCONVERTER_H
