#ifndef CRIODATA_H
#define CRIODATA_H

#include "util/criodefinitions.h"
#include <QString>

class CRioData : public CRIO::Object
{
public:
    QString address;

    CRIO::Timestamp timestamp;

    CRioData(quint8 address, QVariantList data = QVariantList(), CRIO::Timestamp timestamp = CRIO::Timestamp());
    CRioData(QString identifier, QVariantList data = QVariantList(), CRIO::Timestamp timestamp = CRIO::Timestamp());
    // Object interface

    const QVariantList &data() const;
    QList<CRIO::PolymorphicData> polymorphicData() const;
protected:
    QVariantList m_data;
};

#endif // CRIODATA_H
