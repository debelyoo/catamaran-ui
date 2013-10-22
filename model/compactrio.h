#ifndef COMPACTRIO_H
#define COMPACTRIO_H

#include "transformation/abstractcriostatesholder.h"
#include "communication/server.h"
#include "util/criobytearray.h"
#include "communication/criocommand.h"
#include "communication/criodata.h"
#include "communication/criomessage.h"

#include <QQueue>
#include <QPointF>

/** Singleton class that account for the "real" CRio device.
 * @brief The CompactRio class
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

    const QList<NamedAddress> availableInputs() const;
    const QList<Sensor *> selfAllocatedSensors() const;

    // Getters
    qint8 leftEngineValue() const;
    qint8 rightEngineValue() const;
    double heading() const;
    QPointF position() const;
    QPointF speed() const;
    QPointF meanSpeed() const;

    /*
     *  Command Factory methods
     */

    // General Commands
    bool getCommand(const CRIO::CommandAddresses &addr);

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

    // Waypoints Commands
    bool addWaypointCmd(const QPointF &p, int index = -1);
    bool setWaypointsCmd(const QList<QPointF> &points);
    bool delWaypointCmd();

    // Memories
    bool get16bMemory(const CRIO::Memory::CMD_ADDR_16B_BLOC &addr);
    bool set16bMemory(const CRIO::Memory::CMD_ADDR_16B_BLOC &addr, const quint16 &value);

    // PT100 config

    // FGPA counter config
    bool setFpgaCounterSamplingTime(const quint16 &ms);

signals:
    void newCRioStatusMessage(QString msg);
    void enginesChanged();
    void navigationModeChanged();
    void positionChanged();
    void speedChanged();
    void headingChanged();
    void syncTimestampChanged();

private:
    Q_DISABLE_COPY(CompactRio)

    static CompactRio *s_instance;
    Server *m_server;
    SensorConfig *m_sensorConfig;
    CompactRio();

    qint64 m_syncTimestamp;

    CRIO::NAV_SYS_MODE m_navMode;

    qint8 m_leftEngineValue;
    qint8 m_rightEngineValue;
    double m_heading;          // rad / deg ?
    QPointF m_position;
    QPointF m_speed;
    QPointF m_meanSpeed;

    QList<QPointF> m_pastSpeed;

    QList<Sensor *> m_selfAllocatedSensors;
    QList<NamedAddress> m_availableInputs;

    void processMeanSpeed();
    void initSelftAllocatedSensors();
    void initAvailableInputs();

};

#endif // COMPACTRIO_H
