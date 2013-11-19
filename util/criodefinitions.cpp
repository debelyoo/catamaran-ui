#include "criodefinitions.h"
#include <QtCore>

qint64 CRIO::Timestamp::LABVIEW_EPOCH = QDateTime::fromString(QString("1904-01-01T00:00:00"), Qt::ISODate).toMSecsSinceEpoch();
qint64 CRIO::Timestamp::timestampDeltaMs = -5000000000;

/*
 *  Timestamp class definition
 */

/**
 * @brief CRIO::Timestamp::Timestamp default constructor
 */
CRIO::Timestamp::Timestamp():
    timestamp(0)
{
}

/**
 * @brief CRIO::Timestamp::Timestamp constructor using a lavbiew timestamp
 * @param secs
 * @param fracs
 */
CRIO::Timestamp::Timestamp(qint64 secs, quint64 fracs)
{
    timestamp = secs + ((double) fracs) / Q_UINT64_C(18446744073709551615);
    unixTimestamp = toUnixMsTimestamp(timestamp);
}

/**
 * @brief CRIO::Timestamp::Timestamp constructor using a lavbiew timestamp
 * @param ts
 */
CRIO::Timestamp::Timestamp(double ts):
    timestamp(ts)
{
    unixTimestamp = toUnixMsTimestamp(timestamp);
}

/**
 * @brief CRIO::Timestamp::Timestamp copy constructor
 * @param ts
 */
CRIO::Timestamp::Timestamp(const CRIO::Timestamp &ts):
    timestamp(ts.timestamp),
    unixTimestamp(ts.unixTimestamp)
{

}

/**
 * @brief CRIO::Timestamp::toUnixMsTimestamp create a unix timestamp using crio timestamp. This method also add the right offset
 * for timestamp correction
 * @param timestamp in labview epoch
 * @return a unix timestamp
 */
qint64 CRIO::Timestamp::toUnixMsTimestamp(double timestamp)
{
    qint64 newTs = (timestamp*1000)+CRIO::Timestamp::timestampDeltaMs;
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
