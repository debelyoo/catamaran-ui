#ifndef PRISMEMESSAGE_H
#define PRISMEMESSAGE_H

#include <QVariant>
#include <QDebug>
#include <QStringList>

//#include "LabiewTypes.h"
#include "communication/criodata.h"
#include "communication/idatamessagereceiver.h"

class PRismeMessage{
public:

    static int ADDRESS_OFFSET; /**< Offset between addresses on the PRisme and the cRIO. */
    static int PSEUDO_TIMESTAMP_INC_MS; /**< Pseudo TS multiplier in ms that convert PRisme value into real time value. */
    static qint64 PSEUDO_TIMESTAMP_SYNC_VALUE; /**< Pseudo TS sync value. */
    static CRioData decodePrismeData(const CRioData &msg, IDataMessageReceiver *callback, const QString &sensorAddress, const QStringList &subsensorsAddress);
//    static bool dataAvailiable();
private:
    PRismeMessage();
    static QByteArray buffer;
    static qint64 lastPTS;
};

#endif // PRISMEMESSAGE_H
