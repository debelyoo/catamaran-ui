#include "criomessage.h"

#include <QDebug>

/*
 *  CRIOMessage class definition
 */
quint32 CRioMessage::s_neededBytes = 0;
CRIO::MessageType CRioMessage::s_currentType = CRIO::MESSAGE_TYPE_ERROR;

/**
 * @brief CRioMessage::CRioMessage Constructor
 *
 * Create a message by reading bytes in a datastream.
 * If we can, we read the first five bytes from the datastream to get the type of the message
 * and the number of bytes needed to decode the message.
 * If there is enough available bytes in the datastream, we try to build the message
 * according to the type.
 * @param ds the datastream used for creation
 */
CRioMessage::CRioMessage(CRioDataStream &ds):
    m_pContent(NULL),
    m_valid(false)
{
    if((s_neededBytes <= 0) | (s_currentType == CRIO::MESSAGE_TYPE_ERROR)){
        if(ds.device()->bytesAvailable() >= 5){
            quint8 messageType;
            ds >> messageType;
            s_currentType = (CRIO::MessageType) messageType;
            ds >> s_neededBytes;

        }
    }
    m_type = s_currentType;
    if(s_neededBytes <= ds.device()->bytesAvailable()){
        switch(m_type){
        case CRIO::CMD:
            m_valid = buildCommand(ds);
            break;
        case CRIO::DATA:
            m_valid = buildData(ds);
            break;
        default:
            s_currentType = CRIO::MESSAGE_TYPE_ERROR;
            break;
        }
    }
}

/**
 * @brief CRioMessage::~CRioMessage Destructor
 */
CRioMessage::~CRioMessage(){
    if(m_pContent){
        delete m_pContent;
    }
}

/**
 * @brief CRioMessage::type getter for the type
 * @return the type
 */
CRIO::MessageType CRioMessage::type() const{
    return m_type;
}

/**
 * @brief CRioMessage::isValid getter for the validity of the message
 * @return True if message is valid, false otherwise
 */
bool CRioMessage::isValid() const{
    //if(!m_valid)qDebug() << "s_neededB="<<s_neededBytes<<" s_cType="<<s_currentType;
    return m_valid;
}

/**
 * @brief CRioMessage::content getter for the content pointer
 * @return if valid, a pointer on the content, oterwise Null
 */
CRIO::Object *CRioMessage::content()
{
    return m_pContent;
}

/**
 * @brief CRioMessage::reset reset the status of the builder
 *
 * This method is used when errors are reached to re-synchronize the builder and the datastream
 */
void CRioMessage::reset()
{
    s_neededBytes = 0;
    s_currentType = CRIO::MESSAGE_TYPE_ERROR;
}

/**
 * @brief CRioMessage::buildCommand this method try to create a command using the datastream.
 *
 * This method skip unused bytes to avoid a de-synchronization between the builder and the datastream
 * @param ds the datastream used to create a command
 * @return true on success, false otherwise
 */
bool CRioMessage::buildCommand(CRioDataStream &ds)
{

    int pos = ds.device()->pos();
    m_pContent = createCommand(ds);
    int skip = s_neededBytes - (ds.device()->pos()-pos);
    if(skip > 0){       // skip unsued bytes
        ds.skipRawData(skip);
    }
    // reset static variables
    s_neededBytes = 0;
    s_currentType = CRIO::MESSAGE_TYPE_ERROR;
    return true;
}

/**
 * @brief CRioMessage::buildData this method try to create a command using the datastream
 *
 * This method skip unused bytes to avoid a de-synchronization between the builder and the datastream
 * @param ds the datastream
 * @return true on success, false otherwise
 */
bool CRioMessage::buildData(CRioDataStream &ds)
{
    int pos = ds.device()->pos();
    m_pContent = createData(ds);
    int skip = s_neededBytes - (ds.device()->pos()-pos);
    if(skip > 0){       // skip unused bytes
        ds.skipRawData(skip);
    }
    // reset static variables
    s_neededBytes = 0;
    s_currentType = CRIO::MESSAGE_TYPE_ERROR;
    return true;
}

/**
 * @brief CRioMessage::createData create a data content with the datastream.
 *
 * This method create a CRioData by decoding the datastream regarding the structure of a data.
 * For more information, see the [Wiki](http://wiki.epfl.ch/ecol-cat-gui/data-structure "Catamaran Wiki: Data Structure").
 * @param ds the datastream
 * @return the datastream
 */
CRioData *CRioMessage::createData(CRioDataStream &ds)
{
    quint8 address;
    ds >> address;
    QList<CRIO::PolymorphicData> pl;
    ds >> pl;
    QVariantList vl;
    foreach(CRIO::PolymorphicData p, pl){
        vl.append(p.value);
    }
    qint64 secs;
    quint64 fracs;
    ds >> secs;
    ds >> fracs;
    CRIO::Timestamp ts(secs, fracs);

    //ds.skipRawData(nByte);
    return new CRioData(address, vl, ts);
}

/**
 * @brief CRioMessage::createCommand create a command content with the datastream.
 *
 * This method create a CRioData by decoding the datastream regarding the structure of a data.
 * For more information, see the [Wiki page](http://wiki.epfl.ch/ecol-cat-gui/data-structure "Catamaran Wiki: Data Structure") for data structure or this
 * [Wiki page](http://wiki.epfl.ch/ecol-cat-gui/commands "Catamaran Wiki: List of commands") for command explanation
 * @param ds the datastream
 * @return the datastream
 */
CRioCommand *CRioMessage::createCommand(CRioDataStream &ds)
{
    bool error = false;
    quint8 cmdType;
    ds >> cmdType;              // get the command type
    quint32 nParams;
    ds >> nParams;              // get the number of parameters in the command
    QVariantList params;

    /* BEGIN SWITCH on command type */
    switch(cmdType){
    case CRIO::CMD_GET:
    {
        quint8 address;
        quint32 strlen;
        ds >> strlen;           // length in bytes of the next paramater
        ds >> address;          // address for the get command
        return new CRioCommand((CRIO::CommandTypes)cmdType, (CRIO::CommandAddresses)address, params);
    }
        break;
    case CRIO::CMD_SET:
    {
        quint8 address;
        quint32 strlen;
        if(nParams > 0){
            ds >> strlen;
            ds >> address;
            --nParams;
        }
        /* BEGIN SWITCH on address */
        switch(address){
        case CRIO::CMD_ADDR_PRISME_TS_SYNC:
        {
            ds >> strlen;
            qint64 secs;
            quint64 fracs;
            ds >> secs;
            ds >> fracs;
            CRIO::Timestamp ts(secs, fracs);
            params.append(QVariant::fromValue(ts));
            break;
        }
        case CRIO::CMD_ADDR_CURRENT_TIME:
        {
            ds >> strlen;
            qint64 secs;
            quint64 fracs;
            ds >> secs;
            ds >> fracs;
            CRIO::Timestamp ts(secs, fracs);
            params.append(QVariant::fromValue(ts));
            break;
        }
            break;
        case CRIO::CMD_ADDR_HONK:

            break;
        default:
            break;
        }
        /* END SWITCH on address */
        return new CRioCommand((CRIO::CommandTypes)cmdType, (CRIO::CommandAddresses)address, params);
    }
        break;
    case CRIO::CMD_ADD:
    {
        //ds >> address;
    }
        break;
    case CRIO::CMD_DEL:
    {
        //ds >> address;
    }
        break;
    case CRIO::CMD_STOP:
        break;
    case CRIO::CMD_NOTIFY:
    {
        quint32 strlen;
        quint16 notificationType;
        ds >> strlen;
        ds >> notificationType;

        ds >> strlen;
        qint64 secs;
        quint64 fracs;
        ds >> secs;
        ds >> fracs;

        CRIO::Timestamp ts(secs, fracs);
        params.append(QVariant::fromValue(ts));
        /* BEGIN SWITCH on notification type */
        switch(notificationType){
            case CRIO::NOTIFY_NS_END_REACHED:

            break;
        case CRIO::NOTIFY_NS_NEW_LINE:
        {
            quint16 id1, id2;
            ds >> strlen;
            ds >> id1;
            ds >> strlen;
            ds >> id2;
            params.append(QVariant::fromValue(id1));
            params.append(QVariant::fromValue(id2));
            break;
        }
        case CRIO::NOTIFY_NS_WP_REACHED:
        {
            quint16 id;
            ds >> strlen;
            ds >> id;
            params.append(QVariant::fromValue(id));
            break;
        }
        default:
            break;
        }
        /* END SWITCH on notification type */
        return new CRioCommand((CRIO::CommandTypes)cmdType, (CRIO::CommandAddresses)notificationType, params);
        break;
    }
    default:
        break;
    }
    /* END SWITCH on command type */
    if(error){
        return NULL;
    }
    return NULL;
}
