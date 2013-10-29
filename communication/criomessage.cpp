#include "criomessage.h"

#include <QDebug>

/*
 *  CRIOMessage class definition
 */
quint32 CRioMessage::s_neededBytes = 0;
CRIO::MessageType CRioMessage::s_currentType = CRIO::MESSAGE_TYPE_ERROR;

CRioMessage::CRioMessage(CRioDataStream &ds):
    m_pContent(NULL),
    m_valid(false)
{
    if(s_neededBytes <= 0 | s_currentType == CRIO::MESSAGE_TYPE_ERROR){
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

CRioMessage::~CRioMessage(){
    if(m_pContent){
        delete m_pContent;
    }
}

CRIO::MessageType CRioMessage::type() const{
    return m_type;
}

bool CRioMessage::isValid() const{
    if(!m_valid)qDebug() << "s_neededB="<<s_neededBytes<<" s_cType="<<s_currentType;
    return m_valid;
}

CRIO::Object *CRioMessage::content()
{
    return m_pContent;
}

void CRioMessage::reset()
{
    s_neededBytes = 0;
    s_currentType = CRIO::MESSAGE_TYPE_ERROR;
}

bool CRioMessage::buildCommand(CRioDataStream &ds)
{

    int pos = ds.device()->pos();
    m_pContent = createCommand(ds);
    int skip = s_neededBytes - (ds.device()->pos()-pos);
    if(skip > 0){
        ds.skipRawData(skip);
//        int skiped = ds.skipRawData(skip);
//        qDebug() << "build CMD : skip " << skip << " ("<<skiped<<") bytes.";
//        qDebug() << "\tneeded="<<s_neededBytes;
//        qDebug() << "\tpos1="<<pos;
//        qDebug() << "\tpos2="<<ds.device()->pos();
//        qDebug() << "\tab="<<ds.device()->bytesAvailable();
//        CRioCommand *cmd = (CRioCommand *)m_pContent;
//        if(cmd){
//            qDebug() << "\tCMD="<<cmd->command() << " addr="<<cmd->address();
//            foreach(QVariant v, cmd->parameters()){
//                qDebug() << "\t\t"<<v;
//            }
//        }else{
//            qDebug() << "\tCMD invalid";
//        }

        //char *s = new char[skip];
        //ds.readRawData(s, skip);
        //qDebug() << QByteArray(s, skip).toHex();
        //delete [] s;
    }
    s_neededBytes = 0;
    s_currentType = CRIO::MESSAGE_TYPE_ERROR;
    return true;
}

bool CRioMessage::buildData(CRioDataStream &ds)
{
    int pos = ds.device()->pos();
    m_pContent = createData(ds);
    int skip = s_neededBytes - (ds.device()->pos()-pos);
    if(skip > 0){
        ds.skipRawData(skip);
//        int skiped = ds.skipRawData(skip);
//        CRioData *data = (CRioData*) m_pContent;
//        qDebug() << "build DATA : skip " << skip << " ("<<skiped<<") bytes.";
//        qDebug() << "\tneeded="<<s_neededBytes;
//        qDebug() << "\tpos1="<<pos;
//        qDebug() << "\tpos2="<<ds.device()->pos();
//        qDebug() << "\tab="<<ds.device()->bytesAvailable();
//        if(data){
//            qDebug() << "\tData.addr="<<data->address<<", nP="<<data->polymorphicData().count();
//            foreach(CRIO::PolymorphicData p, data->polymorphicData()){
//                qDebug() << "\t\t" << p.value;
//            }
//        }
    }
    s_neededBytes = 0;
    s_currentType = CRIO::MESSAGE_TYPE_ERROR;
    return true;
}

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

CRioCommand *CRioMessage::createCommand(CRioDataStream &ds)
{
    bool error = false;
    quint8 cmdType;
    ds >> cmdType;
    quint32 nParams;
    ds >> nParams;
    QVariantList params;
    quint8 address = 0;
    switch(cmdType){
    case CRIO::CMD_GET:
    {
        quint32 strlen;
        ds >> strlen;
        ds >> address;
    }
        break;
    case CRIO::CMD_SET:
    {
        quint32 strlen;
        if(nParams > 0){
            ds >> strlen;
            ds >> address;
            --nParams;
        }
        switch(address){
        case CRIO::CMD_ADDR_PRISME_TS_SYNC:
        {
            ds >> strlen;
            qint64 secs;
            quint64 fracs;
            ds >> secs;
            ds >> fracs;
            CRIO::Timestamp ts(secs, fracs);
            params.append(QVariant::fromValue(ts.timestamp));
        }
            break;
        case CRIO::CMD_ADDR_HONK:

            break;
        default:
            break;
        }
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
    }

    if(error){
        return NULL;
    }
    return new CRioCommand((CRIO::CommandTypes)cmdType, (CRIO::CommandAddresses)address, params);
}
