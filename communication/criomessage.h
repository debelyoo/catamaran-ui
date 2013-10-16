#ifndef CRIOMESSAGE_H
#define CRIOMESSAGE_H

#include "util/criodefinitions.h"
#include "util/criodatastream.h"
#include "communication/criocommand.h"
#include "communication/criodata.h"

class CRioMessage{
public:
    CRioMessage(CRioDataStream &ds);
    ~CRioMessage();

    CRIO::MessageType type() const;
    bool isValid() const;
    CRIO::Object *content();
protected:
    CRioCommand *createCommand(CRioDataStream &ds);
    CRioData *createData(CRioDataStream &ds);
private:
    CRIO::MessageType m_type;
    CRIO::Object *m_pContent;
    bool m_valid;

    bool buildCommand(CRioDataStream &ds);
    bool buildData(CRioDataStream &ds);

    static CRIO::MessageType s_currentType;
    static quint32 s_neededBytes;

};

#endif // CRIOMESSAGE_H
