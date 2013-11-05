#ifndef CRIODEFINITIONS_H
#define CRIODEFINITIONS_H


#include <QList>
#include <QVariant>
#include <QDataStream>
#include <QPointF>
#include <QList>


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
        SBG_IG_500E_Output = 10,
        Invalide = 255
    } Types;
}

class CRioDataStream;
namespace CRIO {
    typedef enum {  // cast to quint8
        DATA = 0,
        CMD = 1,
        MESSAGE_TYPE_ERROR
    } MessageType;

    typedef enum {
        DECODING_ERROR,
        DECODING_NOT_ENOUGH_BYTE,
        DECODING_SUCCEEDED
    } DECODING_STATUS;

    typedef enum {
        LEFT,
        RIGHT
    } Engines;

    typedef enum {  // cast to quint8
        CMD_STOP = 0,
        CMD_GET = 1,
        CMD_SET = 2,
        CMD_ADD = 3,
        CMD_DEL = 4
    } CommandTypes;


    typedef enum {  // cast to quint8
        CMD_ADDR_NO_ADDRESS = 0,            // Reserved (Error Check)
        CMD_ADDR_MEMORY_8BIT = 1,           // Memory8b
        CMD_ADDR_MEMORY_16BIT = 2,          // Memory16b
        CMD_ADDR_MEMORY_32BIT = 3,          // Memory32b
        CMD_ADDR_CURRENT_TIME = 4,
        CMD_ADDR_SENSOR_CONFIG = 5,         // Address Conf & LUT	P1 Array{{(bool) stream, (uint8) addr}, ...}
        CMD_ADDR_ENGINE_MODE = 6,           // Motors Mode	P1 (uint8) Mode [0: Auto, 1: Manual]
        CMD_ADDR_LEFT_ENGINE = 7,           // Left Motor Value  P1 (int 8) value [-127, 127]
        CMD_ADDR_RIGHT_ENGINE = 8,          // Right Motor Value P1 (int 8) value [-127, 127]
        CMD_ADDR_HONK = 9,                  // Honk	 P1 (bool) On/Off
        CMD_ADDR_LIGHT = 10,                // Light P1 (bool) On/Off
        CMD_ADDR_SABERTOOTH_ENABLE = 11,    // Sabertooth Enable	P1 (bool) On/Off
        CMD_ADDR_SABERTOOTH_CONFIG = 12,    // Sabertooth Config	P1 (uint8) Addr , P2 (8bits) Value
        CMD_ADDR_PRISME_TS_SYNC = 13,       // PRisme Sync Timestamp	[GET] (uint8) Prisme addr, [SET] P1 Timestamp
        CMD_ADDR_NS_LIMITS = 14,            // Nav.Sys. Limits   : (double)	Delta, (double) Epsilon
        CMD_ADDR_NS_CSTS = 15,              // Nav.Sys. Constant : (double) C_perp, (double) C_point, (double) C_aheahD, (double) P
        CMD_ADDR_NS_WAYPOINTS = 16          // Nav.Sys. Waypoints	P1…n waypoint {{(double) X, (double) Y}, ...}
    } CommandAddresses;

    typedef enum {  // cast to quint8
        NAV_SYS_AUTO = 0,
        NAV_SYS_MANUAL = 1
    } NAV_SYS_MODE;

    typedef enum {
        OFF = 0,
        ON = 1
    } ON_OFF;

    typedef enum {
        DATA_ADDR_ORIENTATION = 41,
        DATA_ADDR_POSITION = 48,
        DATA_ADDR_VELOCITY = 49,
        DATA_ADDR_MOTOR_MODE = 50,
        DATA_ADDR_RIGHT_ENGINE_FEEDBACK = 51,
        DATA_ADDR_LEFT_ENGINE_FEEDBACK = 52,
        DATA_ADDR_DEBUG = 60,
        DATA_ADDR_NAV_SYS_LOG = 61,
        DATA_ADDR_NI9870_1_P1 = 64,
        DATA_ADDR_NI9870_2_P1 = 65,
        DATA_ADDR_NI9870_3_P1 = 66,
        DATA_ADDR_NI9870_4_P1 = 67
    } DataAddresses;

    class Object{
    public:
    protected:
        Object(){}
    };

    namespace Memory {
        typedef enum {
        NI9217_1_SAMPLING_PERIOD = 0,          //	25-65535	[ms] (Multiple of 25ms ex. 25, 100, 575)
        NI9217_1_SAMPLING_MODE = 4,            //	0-65535	0=>Low Res. (2.5ms), otherwise=>High Res. (200ms)
        NI9217_2_SAMPLING_PERIOD = 5,          //	25-65535	[ms] (Multiple of 25ms ex. 25, 100, 575)
        NI9217_2_SAMPLING_MODE = 9,            //	0-65535	0=>Low Res. (2.5ms), otherwise=>High Res. (200ms)
        FPGA_COUNTER_1_SAMPLING_TIME = 10      //	0-65535	[ms]
        //FPGA Counter-1/Bound port	0-8	0=>Disabled, [1-8] => [DIO0, DIO7]
        }CMD_ADDR_16B_BLOC;
    }

    typedef enum {
        LOW_RES = 0,
        HIGH_RES = 1
    }PT100_SAMPLEING_RESOLUTION;

    struct Timestamp{
        static qint64 toUnixMsTimestamp(double timestamp);
        double timestamp;
        qint64 unixTimestamp;

        Timestamp();
        Timestamp(qint64 secs, quint64 fraqs);
        Timestamp(double ts);
        //Timestamp(CRioDataStream &ds);

    private:
        static qint64 LABVIEW_EPOCH;
    };

    struct PolymorphicData{
        QVariant value;
        PolymorphicData();
        PolymorphicData(QVariant value);
        DataType::Types cRIOType() const;
    };
}

#endif // CRIODEFINITIONS_H
