#ifndef CRIODATASTREAM_H
#define CRIODATASTREAM_H

#include <QDataStream>
#include "util/criodefinitions.h"
#include "communication/criomessage.h"

class CRioDataStream : public QDataStream
{
public:
    CRioDataStream(QIODevice * d);
    CRioDataStream(QByteArray * a, QIODevice::OpenMode mode);
    CRioDataStream(const QByteArray & a);

    //CRioDataStream &operator>>(CRioMessage &msg);
    CRioDataStream &operator >>(CRIO::PolymorphicData &p);
    CRioDataStream &operator >>(QList<CRIO::PolymorphicData> &pl);

    CRioDataStream &operator >>(CRIO::Timestamp &ts);

    //operator QDataStream &();

    CRioDataStream &operator>>(qint8 & i);
    CRioDataStream &operator>>(bool & i);
    CRioDataStream &operator>>(quint8 & i);
    CRioDataStream &operator>>(quint16 & i);
    CRioDataStream &operator>>(qint16 & i);
    CRioDataStream &operator>>(quint32 & i);
    CRioDataStream &operator>>(qint32 & i);
    CRioDataStream &operator>>(quint64 & i);
    CRioDataStream &operator>>(qint64 & i);
    CRioDataStream &operator>>(float & f);
    CRioDataStream &operator>>(double & f);
    CRioDataStream &operator>>(char *& s);

private:
    template<typename T>
    void invertBytes(T &p);

};

#endif // CRIODATASTREAM_H
