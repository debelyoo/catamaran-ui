#ifndef BYTEARRAYCONVERTER_H
#define BYTEARRAYCONVERTER_H

#include "model/sensor.h"
#include <QObject>
#include <QDataStream>
#include <QVector>
#include <QPoint>
#include "util/criodefinitions.h"

#include <QDebug>

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
        QByteArray byteArrayForCmdParameterClusterOfPoint(QPointF pt);
        QByteArray byteArrayForCmdParameterStreamArray(QList<Sensor*> sensors); // return a byte array with length and values (bool, address)

    public slots:

    private:
        ByteArrayConverter() {}
        ByteArrayConverter(const ByteArrayConverter &);
        ByteArrayConverter& operator=(const ByteArrayConverter &);

        static ByteArrayConverter* m_Instance;
};
/*
inline QByteArray &operator <<(QByteArray &ds, const qint64 &a){
    const quint8 * const p = (const quint8 * const)&a;
    for(int i=0;i<8;++i){
        ds.push_back(p[8-1-i]);
    }
    return ds;
}

inline QByteArray &operator <<(QByteArray &ds, const qint32 &a){
    const quint8 * const p = (const quint8 * const)&a;
    for(int i=0;i<4;++i){
        ds.push_back(p[4-1-i]);
    }
    return ds;
}

inline QByteArray &operator <<(QByteArray &ds, const qint16 &a){
    const quint8 * const p = (const quint8 * const)&a;
    for(int i=0;i<2;++i){
        ds.push_back(p[2-1-i]);
    }
    return ds;
}

inline QByteArray &operator <<(QByteArray &ds, const qint8 &a){
    ds.push_back(a);
    return ds;
}

inline QByteArray &operator <<(QByteArray &ds, const quint64 &a){
    const quint8 * const p = (const quint8 * const)&a;
    for(int i=0;i<8;++i){
        ds.push_back(p[8-1-i]);
    }
    return ds;
}

inline QByteArray &operator <<(QByteArray &ds, const quint32 &a){
    const quint8 * const p = (const quint8 * const)&a;
    for(int i=0;i<4;++i){
        ds.push_back(p[4-1-i]);
    }
    return ds;
}

inline QByteArray &operator <<(QByteArray &ds, const quint16 &a){
    const quint8 * const p = (const quint8 * const)&a;
    for(int i=0;i<2;++i){
        ds.push_back(p[2-1-i]);
    }
    return ds;
}

inline QByteArray &operator <<(QByteArray &ds, const quint8 &a){
    ds.push_back(a);
    return ds;
}

inline QByteArray &operator <<(QByteArray &ds, const double &a){
    const quint8 * const p = (const quint8 * const)&a;
    for(int i=0;i<8;++i){
        ds.push_back(p[8-1-i]);
    }
    return ds;
}

inline QByteArray &operator <<(QByteArray &ba, const QVariant &v){
    switch((QMetaType::Type)v.type()){
    case QMetaType::Bool:
        ba << (quint32) 1;
        ba << (quint8) (v.toBool()?1:0); break;
    case QMetaType::ULongLong:
        ba << (quint64) 8;
        ba << v.toUInt(); break;
    case QMetaType::LongLong:
        ba << (quint64) 8;
        ba << v.toInt(); break;
    case QMetaType::UInt:
        ba << (quint32) 4;
        ba << v.toUInt(); break;
    case QMetaType::Int:
        ba << (quint32) 4;
        ba << v.toInt(); break;
    case QMetaType::UShort:
        ba << (quint32) 2;
        ba << v.value<quint16>(); break;
    case QMetaType::Short:
        ba << (quint32) 2;
        ba << v.value<qint16>(); break;
    case QMetaType::UChar:
        ba << (quint32) 1;
        ba << v.value<quint8>(); break;
    case QMetaType::Char:
        ba << (quint32) 1;
        ba << v.value<qint8>(); break;
    case QMetaType::Double:
        ba << (quint32) 8;
        ba << v.toDouble(); break;
    case QMetaType::QPointF:
    {
        ba << (quint32) 2*8;
        QPointF p(v.toPointF());
        ba << p.x() << p.y();
    }
        break;
    default:
        break;
    }

    return ba;
}

inline QByteArray &operator <<(QByteArray &ba, CRIO::PolymorphicData data){
    ba << data.value;
    switch((QMetaType::Type)data.value.type()){
    case QMetaType::Bool:
        ba << (quint8) DataType::UInt8; break;
    case QMetaType::UInt:
        ba << (quint8) DataType::UInt32; break;
    case QMetaType::Int:
        ba << (quint8) DataType::Int32; break;
    case QMetaType::UShort:
        ba << (quint8) DataType::UInt16; break;
    case QMetaType::Short:
        ba << (quint8) DataType::Int16; break;
    case QMetaType::UChar:
        ba << (quint8) DataType::UInt8; break;
    case QMetaType::Char:
        ba << (quint8) DataType::Int8; break;
    case QMetaType::Double:
        ba << (quint8) DataType::Double; break;
    }
}

inline QString byteArrayToString(const QByteArray &ba){
    QString s;
    for(int i=0; i < ba.size(); ++i){
        s += " " + QString::number((quint8)ba.at(i));
    }
    return s;
}

inline QByteArray &operator <<(QByteArray &ba, const CRIO::Command &cmd){
    uint np = cmd.parameters.count();
    ba << cmd.command;
    ba << (quint32)(np);
    foreach(QVariant p, cmd.parameters){
        ba << p;
    }
    return ba;
}

inline QByteArray &operator <<(QByteArray &ba, const CRIO::Data &data){
    ba << data.address;

    return ba;
}
*/
#endif // BYTEARRAYCONVERTER_H
