#include "criodatastream.h"
#include <QDebug>

CRioDataStream::CRioDataStream(QIODevice * d):
    QDataStream(d)
{

}

CRioDataStream::CRioDataStream(QByteArray * a, QIODevice::OpenMode mode):
    QDataStream(a, mode)
{

}

CRioDataStream::CRioDataStream(const QByteArray & a):
    QDataStream(a)
{

}

CRioDataStream &CRioDataStream::operator >>(CRIO::PolymorphicData &p)
{

    quint8 type;
    *this >> type;
    quint32 nb;
    *this >> nb;
    switch(type){
    case DataType::Double:
    {
        double a;
        (*this) >> a;
        p.value = QVariant::fromValue(a);
    }
        break;
    case DataType::String:
    {
        char *a = new char[nb];
        this->readBytes(a, nb);
        p.value = QVariant::fromValue(QString(a));
        delete a;
    }
        break;
//    case DataType::FXP2410:
//        break;
    case DataType::Int32:
    {
        qint32 a;
        (*this) >> a;
        p.value = QVariant::fromValue(a);
    }
        break;
    case DataType::Int16:
    {
        qint16 a;
        (*this) >> a;
        p.value = QVariant::fromValue(a);
    }
        break;
    case DataType::Int8:
    {
        qint8 a;
        (*this) >> a;
        p.value = QVariant::fromValue(a);
    }
        break;
    case DataType::UInt32:
    {
        quint32 a;
        (*this) >> a;
        p.value = QVariant::fromValue(a);
    }
        break;
    case DataType::UInt16:
    {
        quint16 a;
        (*this) >> a;
        p.value = QVariant::fromValue(a);
    }
        break;
    case DataType::UInt8:
    {
        quint8 a;
        (*this) >> a;
        p.value = QVariant::fromValue(a);
    }
        break;
    case DataType::LightAddrConf:
    {
//        double a;
//        (*this) >> a;
//        p.value = QVariant::fromValue(a);
        this->skipRawData(nb);
    }
        break;
//    case DataType::SBG_IG_500E_Output:

//        break;
    case DataType::Invalide:
        this->skipRawData(nb);
        break;
    default:
        break;
    }
    return *this;
}

CRioDataStream &CRioDataStream::operator >>(QList<CRIO::PolymorphicData> &pl)
{
    quint32 nElem;
    *this >> nElem;
    for(quint32 i=0;i < nElem; ++i){
        CRIO::PolymorphicData p;
        *this >> p;
        pl.append(p);
    }
    return *this;
}

CRioDataStream &CRioDataStream::operator >>(CRIO::Timestamp &ts)
{
    qint64 secs;
    quint64 fracs;
    (*this) >> secs;
    (*this) >> fracs;
    CRIO::Timestamp tmp(secs, fracs);
    ts.timestamp = tmp.timestamp;
    ts.unixTimestamp = tmp.unixTimestamp;
    return *this;
}

CRioDataStream &CRioDataStream::operator>>(qint8 & i){
    ((QDataStream*) this)->operator >>(i);
//    invertBytes(i);
    return *this;
}
CRioDataStream &CRioDataStream::operator>>(bool & i){
    ((QDataStream*) this)->operator >>(i);
//    invertBytes(i);
    return *this;
}
CRioDataStream &CRioDataStream::operator>>(quint8 & i){
    ((QDataStream*) this)->operator >>(i);
//    invertBytes(i);
    return *this;
}
CRioDataStream &CRioDataStream::operator>>(quint16 & i){
    ((QDataStream*) this)->operator >>(i);
    //invertBytes(i);
    return *this;
}
CRioDataStream &CRioDataStream::operator>>(qint16 & i){
    ((QDataStream*) this)->operator >>(i);
    //this->invertBytes(i);
    return *this;
}
CRioDataStream &CRioDataStream::operator>>(quint32 & i){
    ((QDataStream*) this)->operator >>(i);
    //this->invertBytes(i);
    return *this;
}
CRioDataStream &CRioDataStream::operator>>(qint32 & i){
    ((QDataStream*) this)->operator >>(i);
    //this->invertBytes(i);
    return *this;
}
CRioDataStream &CRioDataStream::operator>>(quint64 & i){
    ((QDataStream*) this)->operator >>(i);
    //this->invertBytes(i);
    return *this;
}
CRioDataStream &CRioDataStream::operator>>(qint64 & i){
    ((QDataStream*) this)->operator >>(i);
    //this->invertBytes(i);
    return *this;
}
CRioDataStream &CRioDataStream::operator>>(float & f){
    ((QDataStream*) this)->operator >>(f);
    this->invertBytes(f);
    return *this;
}
CRioDataStream &CRioDataStream::operator>>(double & f){
    ((QDataStream*) this)->operator >>(f);
    //QString str = QString("op >> (double) f=%1").arg(f);
    //this->invertBytes(f);
    //qDebug() << str << " if="<<f;
    return *this;
}
CRioDataStream &CRioDataStream::operator>>(char *& s){
    ((QDataStream*) this)->operator >>(s);
    return *this;
}

template<typename T>
void CRioDataStream::invertBytes(T &p)
{
    T tmp;
    quint8 *ptr1 = (quint8 *)&tmp;
    quint8 *ptr2 = (quint8 *)&p;
    size_t size = sizeof(T);
    for(int i=0;i<size;++i){
        ptr1[i] = ptr2[size-i-1];
    }
    p = tmp;
}

/*
CRioDataStream::operator QDataStream &()
{
    return *(QDataStream *) this;
}
*/

/*
CRioDataStream &CRioDataStream::operator>>(CRioMessage &msg){
    quint8 messageType;
    *this >> messageType;
    switch(messageType){
    case CRIO::CMD:

        break;
    case CRIO::DATA:

        break;
    default:
        break;
    }
}
*/
