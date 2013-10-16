#ifndef IDATAMESSAGERECEIVER_H
#define IDATAMESSAGERECEIVER_H

#include "communication/criodata.h"

class IDataMessageReceiver
{
public:
    IDataMessageReceiver(){

    }

    virtual void handleMessageData(CRioData &obj) = 0;
};

#endif // IDATAMESSAGERECEIVER_H
