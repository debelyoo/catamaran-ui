#ifndef CRIODATA_H
#define CRIODATA_H

#include "util/criodefinitions.h"
#include <QString>

struct CRioData : public CRIO::Object
{
public:
    QString address;
    QList<CRIO::PolymorphicData> data;
    CRIO::Timestamp timestamp;

    CRioData(quint8 address, QVariantList data = QVariantList(), CRIO::Timestamp timestamp = CRIO::Timestamp());
    CRioData(QString identifier, QVariantList data = QVariantList(), CRIO::Timestamp timestamp = CRIO::Timestamp());
    static CRIO::Object *create(CRioDataStream &ds);
    // Object interface
protected:

};

#endif // CRIODATA_H
