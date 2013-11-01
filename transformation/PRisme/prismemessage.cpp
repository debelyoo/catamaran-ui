#include "prismemessage.h"

#define PRISME_ADC_CONFIG true
#define PRISME_ADC_VALUE false

#define PRISME_TYPE_MASK 0b1000000000000000
#define PRISME_ADDR_MASK 0b00011111
#define PRISME_DATA_MASK 0b0000001111111111

int PRismeMessage::ADDRESS_OFFSET = 7;
int PRismeMessage::PSEUDO_TIMESTAMP_INC_MS = 100;
qint64 PRismeMessage::PSEUDO_TIMESTAMP_SYNC_VALUE = -1;
qint64 PRismeMessage::lastPTS = -1;
QByteArray PRismeMessage::buffer = QByteArray();
//IDataMessageReceiver* PRismeMessage::pMessageReceiver = 0;

/**
 * Private constructor. Use static methods for PRisme message decoding
 * @brief PRismeMessage::PRismeMessage
 */
PRismeMessage::PRismeMessage()
{
}

QString toBin(quint16 v){
    QString s="";
    quint16 mask = 1<<15;
    for(int i=0;i<16;++i){
        s += (v&mask)?"1":"0";
        mask >>=1;
    }
    return s;
}

/**
 * Try to decode PRisme Message using the DataObject msg and the buffer of undecoded element.
 * @brief PRismeMessage::decodePrismeData
 * @param msg data object
 * @return true if a element as been decoded, false otherwise
 */
CRioData PRismeMessage::decodePrismeData(const CRioData &msg, IDataMessageReceiver *callback, const QString &sensorAddress, const QStringList &subsensorsAddress){

    buffer.append(msg.data().first().toByteArray());
    //qDebug() << "buffer.size ="<<buffer.count()<< " " << buffer.toHex();
    int len = buffer.size() - 4;
    //unsigned char *bdata = (unsigned char*)buffer.data();
    int i = 0;
    while(i<len){
        if(buffer.at(i+3) == '\r' && buffer.at(i+4) == '\n'){
            // Read the pseudo timestamp (8bits)
            unsigned char pseudoTSchar = buffer.at(i);//bdata[i];
            int pseudoTS =  pseudoTSchar * PSEUDO_TIMESTAMP_INC_MS;

            //                qDebug() << "Byte[0]: "<<(unsigned int)bdata[0];
            //                qDebug() << "Byte[1]: "<<(unsigned int)bdata[1];
            //                qDebug() << "Byte[2]: "<<(unsigned int)bdata[2];

            //++bdata;
            /*
                Extract the "encoded" data 16bits
                    b[15]       : Type flag
                    b[10-14]    : Address (5bits)
                    b[0-9]      : Data (10bits)
            */
            unsigned short encodedValue = buffer.at(i+1);//bdata[1+i];
            encodedValue <<= 8;
            encodedValue += buffer.at(i+2);//bdata[2+i];



            bool type = encodedValue & PRISME_TYPE_MASK;
            unsigned char address = (encodedValue >> 10)&PRISME_ADDR_MASK;
            unsigned short data = encodedValue & PRISME_DATA_MASK;

//            if(address == 7){
//                qDebug() << "EV: " << toBin(encodedValue);
//                qDebug() << " d: " << toBin(data);
//            }
            if(lastPTS > 0 && pseudoTS < lastPTS){
                pseudoTS += 256*PSEUDO_TIMESTAMP_INC_MS;
            }
            lastPTS = pseudoTS;
            if(type == PRISME_ADC_VALUE){
                //address += ADDRESS_OFFSET;
                QVariantList params;
                params.append((quint16)data);
                CRioData dObj(sensorAddress+"."+subsensorsAddress[address], params, msg.timestamp /*PSEUDO_TIMESTAMP_SYNC_VALUE + pseudoTS*/);
                callback->handleDataMessage(dObj);
            }
            i += 5;
        }else{
            ++i;
        }
    }
    //QByteArray ba = buffer.left(i);
    buffer.remove(0,i);
    //qDebug() << ba.toHex() << " size ="<<ba.count() << (decoded?" decoded":" not decoded");
    return msg;
}