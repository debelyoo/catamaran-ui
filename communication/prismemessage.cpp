#include "prismemessage.h"
#include <QVariant>

#define PRISME_ADC_CONFIG true
#define PRISME_ADC_VALUE false

#define PRISME_TYPE_MASK 0b1000000000000000
#define PRISME_ADDR_MASK 0b0111110000000000
#define PRISME_DATA_MASK 0b0000001111111111

/**
 * Private constructor. Use static methods for PRisme message decoding
 * @brief PRismeMessage::PRismeMessage
 */
PRismeMessage::PRismeMessage()
{
}
/**
 * Try to decode PRisme Message using the DataObject msg and the buffer of undecoded element.
 * @brief PRismeMessage::decodePrismeData
 * @param msg data object
 * @return true if a element as been decoded, false otherwise
 */
static bool PRismeMessage::decodePrismeData(DataObject msg){
    if(msg.values.begin() != msg.values.end()){
        buffer.append(msg.values.begin()->first.toByteArray());

        int len = buffer.size()-5;
        unsigned char *bdata = buffer.data();
        for(unsigned int i = 0; i<len; ++i){
            if(buffer.at(i+3) == '\r' && buffer.at((i+4) == '\n')){
                // Read the pseudo timestamp (8bits)
                int pseudoTS = bdata[i] * *PSEUDO_TIMESTAMP_INC_MS;
                ++bdata;
                /*
                Extract the "encoded" data 16bits
                    b[15]       : Type flag
                    b[10-14]    : Address (5bits)
                    b[0-9]      : Data (10bits)
                */
                unsigned short encodedValue = *((unsigned short*)bdata);
                i += 5;
                bool type = encodedValue & PRISME_TYPE_MASK;
                unsigned char address = (encodedValue & PRISME_ADDR_MASK) >> 10;
                unsigned short data = encodedValue & PRISME_DATA_MASK;

                if(lastPTS > 0 && pseudoTS < lastPTS){
                    pseudoTS += 256*PSEUDO_TIMESTAMP_INC_MS;
                }
                if(type == PRISME_ADC_VALUE){
                    address += ADDRESS_OFFSET;
                    DataValues values;
                    values.append(QPair<QVariant, DataType::Types>(QVariant(data), DataType::UInt16));
                    DataObject dObj(address, values, PSEUDO_TIMESTAMP_SYNC_VALUE + pseudoTS);
                    MessageConsumer->handleMessageData(&dObj);
                }
                return true;    // New Message parsed
            }else{
                ++i;
            }
        }
        return false;   // No message parsed
    }
}
