#include "criodefinitions.h"
#include <QtCore>

qint64 CRIO::Timestamp::LABVIEW_EPOCH = QDateTime::fromString(QString("1904-01-01T00:00:00"), Qt::ISODate).toMSecsSinceEpoch();

/*
 *  Timestamp class definition
 */
CRIO::Timestamp::Timestamp():
    timestamp(0)
{
}

CRIO::Timestamp::Timestamp(qint64 secs, quint64 fracs)
{
    timestamp = secs + ((double) fracs) / Q_UINT64_C(18446744073709551615);
    unixTimestamp = toUnixMsTimestamp(timestamp);
}

CRIO::Timestamp::Timestamp(double ts):
    timestamp(ts)
{
    unixTimestamp = toUnixMsTimestamp(timestamp);
}

CRIO::Timestamp::Timestamp(CRioDataStream &ds)
{
    qint64 secs;
    quint64 fracs;
    ds >> secs;
    (*(QDataStream *) &ds) >> fracs;
    timestamp = secs + ((double) fracs) / Q_UINT64_C(18446744073709551615);
    unixTimestamp = toUnixMsTimestamp(timestamp);
}

qint64 CRIO::Timestamp::toUnixMsTimestamp(double timestamp)
{
    qint64 newTs = CRIO::Timestamp::LABVIEW_EPOCH + round(timestamp * 1000);
    return newTs;
}

/*
 *  PolymorphicData class definition
 */
CRIO::PolymorphicData::PolymorphicData():
    value()
{

}

CRIO::PolymorphicData::PolymorphicData(QVariant value): value(value){}
DataType::Types CRIO::PolymorphicData::cRIOType() const{
    switch((QMetaType::Type)value.type()){
    case QMetaType::Bool:
        return DataType::UInt8;
    case QMetaType::UInt:
        return DataType::UInt32;
    case QMetaType::Int:
        return DataType::Int32;
    case QMetaType::UShort:
        return DataType::UInt16;
    case QMetaType::Short:
        return DataType::Int16;
    case QMetaType::UChar:
        return DataType::UInt8;
    case QMetaType::Char:
        return DataType::Int8;
    case QMetaType::Double:
        return DataType::Double;
    case QMetaType::QString:
        return DataType::String;
    default:
        return DataType::Invalide;
    }
}
