#include "criodata.h"

/*
 *  Data class definition
 */
CRioData::CRioData(quint8 address, QVariantList data, CRIO::Timestamp timestamp):
    CRIO::Object(),
    m_data(),
    timestamp(timestamp)
{
   this->address = QString::number(address);
    foreach(QVariant v, data){
        this->m_data.append(v);
    }
}

CRioData::CRioData(QString identifier, QVariantList data, CRIO::Timestamp timestamp):
    CRIO::Object(),
    address(identifier),
    m_data(),
    timestamp(timestamp)
{
    foreach(QVariant v, data){
        this->m_data.append(v);
    }
}

const QVariantList &CRioData::data() const
{
    return m_data;
}

QList<CRIO::PolymorphicData> CRioData::polymorphicData() const
{
    QList<CRIO::PolymorphicData> d;
    foreach(QVariant v, m_data){
        d.append(CRIO::PolymorphicData(v));
    }
    return d;
}



