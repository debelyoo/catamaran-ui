#include "criobytearray.h"
#include <QDebug>
#include "model/sensor.h"

CRioByteArray::CRioByteArray():
    m_byteArray()
{
}

CRioByteArray::CRioByteArray(const CRioCommand &cmd):
    m_byteArray()
{
    push(cmd);
}

CRioByteArray::CRioByteArray(const CRioData &data):
    m_byteArray()
{
    push(data);
}

void CRioByteArray::push(const CRioByteArray &a, bool flatten)
{
    if(flatten){
        push((quint32) a.size());
    }
    m_byteArray.append(a.byteArray());
}

void CRioByteArray::push(const QByteArray &a, bool flatten)
{
    if(flatten){
        push((quint32) a.size());
    }
    m_byteArray.append(a);
}

void CRioByteArray::push(const qint8 &a, bool flatten)
{
    if(flatten){
        push((quint32) 1);
    }
    m_byteArray.push_back(a);
}

void CRioByteArray::push(const quint8 &a, bool flatten)
{
    if(flatten){
        push((quint32) 1);
    }
    m_byteArray.push_back(a);
}

void CRioByteArray::push(const qint16 &a, bool flatten)
{
    if(flatten){
        push((quint32) 2);
    }
    const quint8 * const p = (const quint8 * const)&a;
    for(int i=0;i<2;++i){
        m_byteArray.push_back(p[2-1-i]);
    }
}

void CRioByteArray::push(const quint16 &a, bool flatten)
{
    if(flatten){
        push((quint32) 2);
    }
    const quint8 * const p = (const quint8 * const)&a;
    for(int i=0;i<2;++i){
        m_byteArray.push_back(p[2-1-i]);
    }
}

void CRioByteArray::push(const qint32 &a, bool flatten)
{
    if(flatten){
        push((quint32) 4);
    }
    const quint8 * const p = (const quint8 * const)&a;
    for(int i=0;i<4;++i){
        m_byteArray.push_back(p[4-1-i]);
    }
}

void CRioByteArray::push(const quint32 &a, bool flatten)
{
    if(flatten){
        push((quint32) 4);
    }
    const quint8 * const p = (const quint8 * const)&a;
    for(int i=0;i<4;++i){
        m_byteArray.push_back(p[4-1-i]);
    }
}

void CRioByteArray::push(const qint64 &a, bool flatten)
{
    if(flatten){
        push((quint32) 8);
    }
    const quint8 * const p = (const quint8 * const)&a;
    for(int i=0;i<8;++i){
        m_byteArray.push_back(p[8-1-i]);
    }
}

void CRioByteArray::push(const quint64 &a, bool flatten)
{
    if(flatten){
        push((quint32) 8);
    }
    const quint8 * const p = (const quint8 * const)&a;
    for(int i=0;i<8;++i){
        m_byteArray.push_back(p[8-1-i]);
    }
}

void CRioByteArray::push(const double &a, bool flatten)
{
    if(flatten){
        push((quint32) 8);
    }
    const quint8 * const p = (const quint8 * const)&a;
    for(int i=0;i<8;++i){
        m_byteArray.push_back(p[8-1-i]);
    }
}

void CRioByteArray::push(const float &a, bool flatten)
{
    if(flatten){
        push((quint32) 4);
    }
    const quint8 * const p = (const quint8 * const)&a;
    for(int i=0;i<4;++i){
        m_byteArray.push_back(p[4-1-i]);
    }
}

void CRioByteArray::push(const bool &a, bool flatten)
{
    push((quint8) (a?1:0), flatten);
}

void CRioByteArray::push(const QPointF &p, bool flatten)
{
    if(flatten){
        push((quint32) 2*8);
    }
    push(p.x(), false);
    push(p.y(), false);
}

void CRioByteArray::push(const QVariant &v, bool flatten){
    //qDebug() << "push QVariant : ("<<v.typeName()<<"/"<<v.type()<<")" << v;
    switch((QMetaType::Type)v.type()){
    case QMetaType::Bool:
        push((quint8) (v.toBool()?1:0), flatten); break;
    case QMetaType::ULongLong:
        push(v.toULongLong(), flatten); break;
    case QMetaType::LongLong:
        push(v.toLongLong(), flatten); break;
    case QMetaType::UInt:
        push(v.toUInt(), flatten); break;
    case QMetaType::Int:
        push(v.toInt(), flatten); break;
    case QMetaType::UShort:
        push(v.value<quint16>(), flatten); break;
    case QMetaType::Short:
        push(v.value<qint16>(), flatten); break;
    case QMetaType::UChar:
        push(v.value<quint8>(), flatten); break;
    case QMetaType::Char:
        push(v.value<qint8>(), flatten); break;
    case QMetaType::SChar:
        push(v.value<qint8>(), flatten); break;
    case QMetaType::Double:
        push(v.toDouble(), flatten); break;
    case QMetaType::QPointF:
        push(v.toPointF(), flatten); break;
    case QMetaType::QString:
        push(v.toByteArray(), flatten); break;
    case QMetaType::User:
        //qDebug() << "CBA << QV, UserType="<<v.userType()<<" qmt="<<qMetaTypeId<QList<Sensor *> >();
        if(v.userType() == qMetaTypeId<QList<Sensor *> >()){
            push(v.value<QList<Sensor *> >(), flatten);
        }
        break;
    default:
        qDebug() << "Unregistered type("<<v.typeName()<<")";
        break;
    }
}

void CRioByteArray::push(const CRIO::PolymorphicData &v, bool flatten){
    CRioByteArray cba;

    DataType::Types type = v.cRIOType();

    cba.push(v.value, true);
    cba.push((quint8) type);
    if(flatten){
        push((quint32) cba.size());
    }
    push(cba);
}

void CRioByteArray::push(const CRIO::Timestamp &ts, bool flatten)
{
    qint64 secs = (qint64) ts.timestamp;
    quint64 fracs = (quint64)((ts.timestamp - secs) * Q_UINT64_C(18446744073709551615));
    if(flatten){
        push((quint32) 16);
    }
    push(secs);
    push(fracs);
}

void CRioByteArray::push(const QList<Sensor *> sensors, bool flatten)
{
    CRioByteArray cba;

    quint32 len = 0;
    foreach (Sensor* s, sensors) {
        if (s->stream())
        {
            cba.push(true);
            cba.push((quint8)s->address().toInt());
            ++len;
        }
    }
    if(flatten){
        push((quint32) cba.size()+4);
    }
    push(len);
    push(cba);
}

void CRioByteArray::push(const CRioCommand &cmd){
    CRioByteArray cba;
    uint np = cmd.parameters().count();
    cba.push((quint8)cmd.command());
    cba.push((quint32)(np));
    foreach(QVariant p, cmd.parameters()){
        cba.push(p, true);
    }
    push((quint8) CRIO::CMD);
    push(cba, true);
}

void CRioByteArray::push(const CRioData &data){
    CRioByteArray cba;
    uint np = data.data().count();
    cba.push((quint8)data.address.toUShort());
    cba.push((quint32)(np));
    foreach(QVariant d, data.data()){
        cba.push(CRIO::PolymorphicData(d), false);
    }
    cba.push(data.timestamp);

    push((quint8) CRIO::DATA);
    push(cba, true);
}

int CRioByteArray::size() const
{
    return m_byteArray.size();
}

QByteArray &CRioByteArray::operator <<(QByteArray &ba)
{
    ba.push_back(m_byteArray);
    return ba;
}

const QByteArray &CRioByteArray::byteArray() const
{
    return m_byteArray;
}

QByteArray &operator <<(QByteArray &ba, const CRioByteArray &cba)
{
    ba.push_back(cba.byteArray());
    return ba;
}


QDataStream &operator <<(QDataStream &ds, const CRioByteArray &cba)
{
    ds << cba.byteArray();
    return ds;
}
