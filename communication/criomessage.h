#ifndef CRIOMESSAGE_H
#define CRIOMESSAGE_H

#include "util/criodefinitions.h"
#include "util/criodatastream.h"
#include "communication/criocommand.h"
#include "communication/criodata.h"

/**
 * @brief Class used to create CRioCommand or CRioData object using a datastream
 */
class CRioMessage{
public:
    CRioMessage(CRioDataStream &ds);
    ~CRioMessage();

    CRIO::MessageType type() const;
    bool isValid() const;
    CRIO::Object *content();

    static void reset();
protected:
    CRioCommand *createCommand(CRioDataStream &ds);
    CRioData *createData(CRioDataStream &ds);
private:
    bool buildCommand(CRioDataStream &ds);
    bool buildData(CRioDataStream &ds);

    CRIO::MessageType m_type;                   /// Type of the message [CMD / DATA]
    CRIO::Object *m_pContent;                   /// Pointer on the message content (CRioCommand * / CRioData *)
    bool m_valid;                               /// True if content is valid, false otherwise

    static CRIO::MessageType s_currentType;     /// Used for message creation. It contains the type of the pending message
    static quint32 s_neededBytes;               /// Used for message creation. It contains the number of bytes needed to parse the pending message

};

#endif // CRIOMESSAGE_H
