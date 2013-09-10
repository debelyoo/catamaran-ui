#ifndef IDATAMESSAGERECEIVER_H
#define IDATAMESSAGERECEIVER_H

#include "dataObject.h"

class IDataMessageReceiver
{
public:
    IDataMessageReceiver(){

    }

    virtual void handleMessageData(DataObject obj) = 0;
};

#endif // IDATAMESSAGERECEIVER_H
