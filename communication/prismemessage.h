#ifndef PRISMEMESSAGE_H
#define PRISMEMESSAGE_H

#include <QQueue>

//#include "LabiewTypes.h"
#include "dataObject.h"
#include "messageConsumer.h"

class PRismeMessage
{
public:

    static int ADDRESS_OFFSET = 7; /**<
    static int PSEUDO_TIMESTAMP_INC_MS = 10;
    static qint64 PSEUDO_TIMESTAMP_SYNC_VALUE = 0;
    static MessageConsumer* MessageConsumer;
    static bool decodePrismeData(DataObject msg);
//    static bool dataAvailiable();
private:
    PRismeMessage();
    static QByteArray buffer;
    static qint64 lastPTS = -1;
};

#endif // PRISMEMESSAGE_H
