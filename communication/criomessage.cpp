#include "criomessage.h"

#include <QDebug>

/*
 *  CRIOMessage class definition
 */
quint32 CRioMessage::s_neededBytes = 0;
CRIO::MessageType CRioMessage::s_currentType = CRIO::CMD;

CRioMessage::CRioMessage(CRioDataStream &ds):
    m_pContent(NULL),
    m_valid(false)
{
    if(s_neededBytes <= 0){
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
    return m_valid;
}

CRIO::Object *CRioMessage::content()
{
    return m_pContent;
}

bool CRioMessage::buildCommand(CRioDataStream &ds)
{

    int pos = ds.device()->pos();
    m_pContent = CRioCommand::create(ds);
    int skip = s_neededBytes - (ds.device()->pos()-pos);
    //qDebug() << "build CMD : skip " << skip << " bytes.";
    ds.skipRawData(skip);
    s_neededBytes = 0;
    return true;
}

bool CRioMessage::buildData(CRioDataStream &ds)
{
    int pos = ds.device()->pos();
    m_pContent = CRioData::create(ds);
    int skip = s_neededBytes - (ds.device()->pos()-pos);
    //qDebug() << "build DATA : skip " << skip << " bytes.";
    ds.skipRawData(skip);
    s_neededBytes = 0;
    return true;
}
