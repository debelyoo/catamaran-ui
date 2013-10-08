#include "criodata.h"

/*
 *  Data class definition
 */
CRioData::CRioData(quint8 address, QVariantList data, CRIO::Timestamp timestamp):
    CRIO::Object(),
    data(),
    timestamp(timestamp)
{
   this->address = QString::number(address);
    foreach(QVariant v, data){
        this->data.append(v);
    }
}

CRioData::CRioData(QString identifier, QVariantList data, CRIO::Timestamp timestamp):
    CRIO::Object(),
    address(identifier),
    data(),
    timestamp(timestamp)
{
    foreach(QVariant v, data){
        this->data.append(v);
    }
}


CRIO::Object *CRioData::create(CRioDataStream &ds)
{

    quint8 address;
    ds >> address;
    QList<CRIO::PolymorphicData> pl;
    ds >> pl;
    QVariantList vl;
    foreach(CRIO::PolymorphicData p, pl){
        vl.append(p.value);
    }
    CRIO::Timestamp ts;
    ds >> ts;


    //ds.skipRawData(nByte);
    return new CRioData(address, vl, ts);
}
