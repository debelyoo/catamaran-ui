#ifndef CRIODEFINITIONS_H
#define CRIODEFINITIONS_H


#include <QList>
#include <QVariant>
#include <QDataStream>
#include <QPointF>
#include <QList>

/**
 * @file
 */


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

/**
 * @brief CRIO namespace contains the definitions of all items specific to the catamaran
 */
namespace CRIO {
    /// Type of network message
    typedef enum {  // cast to quint8
        DATA = 0,           ///< Stand for a CRioData message
        CMD = 1,            ///< Stand for a CRioCommand message
        MESSAGE_TYPE_ERROR  ///< Point out an error in the decoding process
    } MessageType;

    typedef enum {
        DECODING_ERROR,
        DECODING_NOT_ENOUGH_BYTE,
        DECODING_SUCCEEDED
    } DECODING_STATUS;

    /// Engine selector
    typedef enum {  
        LEFT,       ///< Catamaran left engine
        RIGHT       ///< Catamaran ight engine
    } Engines;

    /// Command types
    typedef enum {  // cast to quint8 
        CMD_STOP = 0,       ///< STOP command, stop engines on the catamaran
        CMD_GET = 1,        ///< GET command, request the value of a specific variable
        CMD_SET = 2,        ///< SET command, set the value of a specific variable
        CMD_ADD = 3,        ///< ADD command, add an element to a specific array
        CMD_DEL = 4,        ///< DEL command, delete an element in a specific array
        CMD_NOTIFY = 5      ///< NOTIFY command, notify a special event
    } CommandTypes;

    /// Command common addresses
    typedef enum {  // cast to quint8 
        CMD_ADDR_NO_ADDRESS = 0,            ///< Reserved (Error Check)
        CMD_ADDR_MEMORY_8BIT = 1,           ///< Memory8b
        CMD_ADDR_MEMORY_16BIT = 2,          ///< Memory16b
        CMD_ADDR_MEMORY_32BIT = 3,          ///< Memory32b
        CMD_ADDR_CURRENT_TIME = 4,          ///< Used for timestamp synchronization : ts (CRIO::Timestamp)
        CMD_ADDR_SENSOR_CONFIG = 5,         ///< Address Conf & LUT	P1 Array{{(bool) stream, (uint8) addr}, ...)
        CMD_ADDR_ENGINE_MODE = 6,           ///< Motors Mode	P1 (uint8) Mode [0: Auto, 1: Manual]
        CMD_ADDR_LEFT_ENGINE = 7,           ///< Left Motor Value  P1 (int 8) value [-127, 127]
        CMD_ADDR_RIGHT_ENGINE = 8,          ///< Right Motor Value P1 (int 8) value [-127, 127]
        CMD_ADDR_HONK = 9,                  ///< Honk	 P1 (bool) On/Off
        CMD_ADDR_LIGHT = 10,                ///< Light P1 (bool) On/Off
        CMD_ADDR_SABERTOOTH_ENABLE = 11,    ///< Sabertooth Enable	P1 (bool) On/Off
        CMD_ADDR_SABERTOOTH_CONFIG = 12,    ///< Sabertooth Config	P1 (uint8) Addr , P2 (8bits) Value
        CMD_ADDR_PRISME_TS_SYNC = 13,       ///< PRisme Sync Timestamp	[GET] (uint8) Prisme addr, [SET] P1 Timestamp
        CMD_ADDR_NS_LIMITS = 14,            ///< Nav.Sys. Limits   : (double)	Delta, (double) Epsilon
        CMD_ADDR_NS_CSTS = 15,              ///< Nav.Sys. Constant : (double) C_perp, (double) C_point, (double) C_aheahD, (double) P
        CMD_ADDR_NS_WAYPOINTS = 16,         ///< Nav.Sys. Waypoints : P1…n waypoint {{(double) X, (double) Y}, ...}
        CMD_ADDR_NS_SPEED_SETPOINT = 17     ///< Nav.Sys. Speed Setpoint : v in m/s (double)
    } CommandAddresses;

    /// Notification types
    typedef enum {
        NOTIFY_ERROR = 0,                   ///< Reserved (Error Check)
        NOTIFY_NS_STARTED = 1,              ///< Navigation system started
        NOTIFY_NS_STOPPED = 2,              ///< Navigation system stopped
        NOTIFY_NS_WP_REACHED = 3,           ///< Navigation system, Catamaran reached a waypoint : (uint16) WP id
        NOTIFY_NS_NEW_LINE = 4,             ///< Navigation system, new line in use : (uint16) WP[n] id, (uint16) WP[n+1] id
        NOTIFY_NS_END_REACHED = 5,          ///< Navigation system, end of the path reached
        NOTIFY_CATAMARAN_STOPPED = 6        ///< Catamaran stopped (with STOP command)
    } NotificationTypes;

    /// Navigation modes
    typedef enum {  // cast to quint8
        NAV_SYS_AUTO = 0,                   ///< Navigation system enabled
        NAV_SYS_MANUAL = 1                  ///< Navigation system disabled
    } NAV_SYS_MODE;

    typedef enum {
        OFF = 0,                            ///< Stand for off/disabled
        ON = 1                              ///< Stand for on/enabled
    } ON_OFF;

    /// Data addresses
    typedef enum {  
        DATA_ADDR_ORIENTATION = 41,             ///< Heading address
        DATA_ADDR_POSITION = 48,                ///< Position address
        DATA_ADDR_VELOCITY = 49,                ///< Veolcity address
        DATA_ADDR_MOTOR_MODE = 50,              ///< Navigation mode address
        DATA_ADDR_RIGHT_ENGINE_FEEDBACK = 51,   ///< Right engine feedback address
        DATA_ADDR_LEFT_ENGINE_FEEDBACK = 52,    ///< Left engine feedback address
        DATA_ADDR_DEBUG = 60,                   ///< Debug message address
        DATA_ADDR_NAV_SYS_LOG = 61,             ///< Navigation system debug message address
        DATA_ADDR_NI9870_1_P1 = 64,             ///< Serial module, port 1 address
        DATA_ADDR_NI9870_2_P1 = 65,             ///< Serial module, port 2 address
        DATA_ADDR_NI9870_3_P1 = 66,             ///< Serial module, port 3 address
        DATA_ADDR_NI9870_4_P1 = 67              ///< Serial module, port 4 address
    } DataAddresses;

    /**
     * @brief The Object class is the base class for CRioData and CRioCommand classes
     */
    class Object{
    public:
    protected:
        Object(){}
    };

    /**
     * @brief Memory namespace countains useful declaration for crio internal memory
     */
    namespace Memory {
        /// Addresses for 16 bits memory block (256 addresses available)
        typedef enum {
            NI9217_1_SAMPLING_PERIOD = 0,       ///<	25-65535	[ms] (Multiple of 25ms ex. 25, 100, 575)
            NI9217_1_SAMPLING_MODE = 4,         ///<	0-65535	0=>Low Res. (2.5ms), otherwise=>High Res. (200ms)
            NI9217_2_SAMPLING_PERIOD = 5,       ///<	25-65535	[ms] (Multiple of 25ms ex. 25, 100, 575)
            NI9217_2_SAMPLING_MODE = 9,         ///<	0-65535	0=>Low Res. (2.5ms), otherwise=>High Res. (200ms)
            FPGA_COUNTER_1_SAMPLING_TIME = 10   ///<	0-65535	[ms]
            //FPGA Counter-1/Bound port	0-8	0=>Disabled, [1-8] => [DIO0, DIO7]
        }CMD_ADDR_16B_BLOC;
    }

    /// PT100 module sampling modes
    typedef enum {
        /// Stand for low resolution mode (high sampling rate ~2.5ms per channel)
        LOW_RES = 0,
        /// Stand for high resolution mode (low sampling rate ~200ms per channel)
        HIGH_RES = 1
    }PT100_SAMPLEING_RESOLUTION;

    /**
     * @brief The Timestamp struct is used to create a universal time representation
     * either from labview timestamp or from unix timestamp. It is also used to syncronized
     * timestamp coming from the crio on the local machine time (it correct time error and timezone error)
     */
    struct Timestamp{
        static qint64 toUnixMsTimestamp(double timestamp);
        double timestamp;       ///< labview representation in seconds
        qint64 unixTimestamp;   ///< unix (synchronized) representation in miliseconds

        Timestamp();
        Timestamp(qint64 secs, quint64 fraqs);
        Timestamp(double ts);
        Timestamp(const Timestamp &ts);
        //Timestamp(CRioDataStream &ds);

        static qint64 timestampDeltaMs; ///< static variable, set by a CompactRio instance, used to synchronize timestamps

    private:
        static qint64 LABVIEW_EPOCH;    ///< not used anymore
    };

    /**
     * @brief The PolymorphicData struct is used to represent data as the crio does. A type value is embeded in the structure
     * to ease encoding and decoding process.
     */
    struct PolymorphicData{
        QVariant value;
        PolymorphicData();
        PolymorphicData(QVariant value);
        DataType::Types cRIOType() const;
    };
}

Q_DECLARE_METATYPE(CRIO::Timestamp)

#endif // CRIODEFINITIONS_H
