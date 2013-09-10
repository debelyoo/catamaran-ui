#ifndef DEFINITIONS_H
#define DEFINITIONS_H

namespace DataType {
    typedef enum {
        Double = 0,
        String = 1,
        FXP2410 = 2,
        Int32 = 3,
        Int16 = 4,
        Int8 = 5,
        UInt32 = 6,
        UInt16 = 7,
        UInt8 = 8,
        LightAddrConf = 9,
        SBG_IG_500E_Output = 10
    } Types;
}

class DataObject;
typedef void (*DataObjectInjectionFunction)(DataObject);

#endif // DEFINITIONS_H
