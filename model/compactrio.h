#ifndef COMPACTRIO_H
#define COMPACTRIO_H

#include "transformation/transformationbaseclass.h"
#include "transformation/abstractcriostatesholder.h"
#include "communication/server.h"
#include "util/criobytearray.h"
#include "communication/criocommand.h"
#include "communication/criodata.h"
#include "communication/criomessage.h"

#include <QQueue>
#include <QPointF>

/**
 * @brief The CompactRio class
 * Singleton class that account for the "real" CRio device. The CompactRio instance is a local copy of the "real" CRio.
 * It contains its states and has some method to change these
 */
class CompactRio : public QObject, public AbstractCrioStatesHolder
{
    Q_OBJECT
public:
    struct NamedAddress{
        quint8 address;
        QString name;
        NamedAddress(quint8 addr, QString n){
            address = addr;
            name = n;
        }
    };

    static CompactRio *instance();

    void feedWithData(const CRioData &data);
    void feedWithCommand(const CRioCommand &cmd);

    const QList<QPointF> &waypoints() const;
    QPointF waypoint(int index) const;

    int addWaypoint(const QPointF &wp);
    bool removeWaypoint(int index);
    void removeLastWaypoint();
    void clearWaypoint(bool force = false);

    const QList<NamedAddress> availableInputs() const;
    const QList<Sensor *> selfAllocatedSensors() const;

    // Getters
    qint8 leftEngineValue() const;
    qint8 rightEngineValue() const;
    double heading() const;
    QPointF position() const;
    QPointF speed() const;
    QPointF meanSpeed() const;

    qint64 crioSyncTimestamp() const;
    qint64 localSyncTimestamp() const;

    bool timesampSynchronized() const;

    void resetTimestampSynchronization();

    /*
     *  Command Factory methods :
     *  Those methods are used as helper/tool to build and send common command to the crio
     */

    // General Commands
    bool getCommand(const CRIO::CommandAddresses &addr);
    bool getCurrentTime();

    // Engine Commands
    bool setEngine(const CRIO::Engines engine, const qint8 value);
    bool stop();

    // Sensor and device Commands
    bool setSensorsConfig();
    bool setHonk(const CRIO::ON_OFF &mode);
    bool setLight(const CRIO::ON_OFF &mode);
    bool setSabertoothState(const CRIO::ON_OFF &mode);
    bool setSabertoothConfig(const quint8 &configAddr, const quint8 &value);

    // Navigation system command
    bool setNavSysMode(const CRIO::NAV_SYS_MODE &mode);
    bool setNavSysConstants(const double C_perp, const double C_point, const double C_aheadD, const double Kp_Y, const double Kp_V);
    bool setNavSysLimits(const double delta, const double epsilon);
    bool setNavSysSpeedSetpoint(const double setpoint);

    // Waypoints Commands
    bool addWaypointCmd(const QPointF &p, int index = -1);
    bool setWaypointsCmd(const QList<QPointF> &points);
    bool delWaypointCmd();

    // Memories
    bool get16bMemory(const CRIO::Memory::CMD_ADDR_16B_BLOC &addr);
    bool set16bMemory(const CRIO::Memory::CMD_ADDR_16B_BLOC &addr, const quint16 &value);

    // PT100 config

    // PRisme config
    bool setPRismeSamplingRate(quint8 n100ms, quint8 portAddress);
    bool getPRismeSyncTimestamp();

    // FGPA counter config
    bool setFpgaCounterSamplingTime(const quint16 &ms);

    // Notification
    void notificationNavSysNewLinePointIndexes(quint16 id1, quint16 id2, const CRIO::Timestamp &ts);
    void notificationNavSysWaypointReached(quint16 id2, const CRIO::Timestamp &ts);
    /*
     * End of Command Factory methods
     */

signals:
    void newCRioStatusMessage(QString msg);     // triggered for crio status notification
    void enginesChanged();                      // triggered when engines value change
    void navigationModeChanged();               // triggered when navigation mode change
    void positionChanged();                     // triggered when position change
    void speedChanged();                        // triggered when speed change
    void headingChanged();                      // triggered when heading change
    void syncTimestampChanged();                // triggered when sync timestamp change
    void nonConsecutiveDataReceived();          // triggered when data with big timestamp difference arrives
    void navSysNewLineInUse();
    void navSysWaypointReached(quint16 index);

private:
    Q_DISABLE_COPY(CompactRio)                  // disable copy constructor (setting it private) beacause this class is a singleton

    static CompactRio *s_instance;              // static instance of the singleton (~=this)
    Server *m_server;                           // poiter on Server singleton
    SensorConfig *m_sensorConfig;               // pointer on SensorConfig singleton
    CompactRio();                               // private constructor 'cause singleton

    double m_crioTimestamp;                     // timestamp on the cRio, used for timestamp synchronization
    qint64 m_localTimestamp;                    // local timestamp, used for timestamp synchronization

    CRIO::NAV_SYS_MODE m_navMode;               // local storage of navigation mode (auto sync with crio)

    QList<QPointF> m_waypoints;                 // local storage of waypoints
    QPair<quint16, quint16> m_waypointsUsedInLineIndexes;   // waypoint currently used by the navigation system


    qint8 m_leftEngineValue;                    // local storage of the left engine value  (auto sync with crio)
    qint8 m_rightEngineValue;                   // local storage of the right engine value (auto sync with crio)
    double m_heading;                           // local storage of the catamaran heading (auto sync with crio)    // rad / deg ?
    QPointF m_position;                         // local storage of the current position (auto sync with crio)
    QPointF m_speed;                            // local storage of the current speed (auto sync with crio)
    QPointF m_meanSpeed;                        // mean of the speed on the tenth past speed values

    QList<QPointF> m_pastSpeed;                 // list of past speed valued (max 10)

    QList<Sensor *> m_selfAllocatedSensors;     // list of all immutable sensor present on the catamran (like GPS, compass, engines values, ...)
    QList<NamedAddress> m_availableInputs;      // list of all available sensor inputs on the catamaran (like PT100, serials, ...)

    bool m_currentTimeIsSet;                    // flag used for validity checking on incomming datas
    CRIO::Timestamp m_lastTimestamp;            // timestamp used for discontinuities / disconnections tracking

    void processMeanSpeed();
    void initSelftAllocatedSensors();
    void initAvailableInputs();
};

#endif // COMPACTRIO_H
