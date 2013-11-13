#include "compactrio.h"
#include "manager/sensortypemanager.h"
#include "transformation/PRisme/prisme.h"

#include <QDebug>

CompactRio *CompactRio::s_instance = NULL;

/**
 * @brief CompactRio::CompactRio Default contstructor (private)
 */
CompactRio::CompactRio():
    QObject(),
    AbstractCrioStatesHolder(),
    m_server(Server::instance()),
    m_sensorConfig(SensorConfig::instance()),
    m_crioTimestamp(),
    m_navMode(),
    m_leftEngineValue(),
    m_rightEngineValue(),
    m_heading(),
    m_position(),
    m_speed(),
    m_meanSpeed(),
    m_pastSpeed(),
    m_selfAllocatedSensors(),
    m_currentTimeIsSet(false)
{
    initSelftAllocatedSensors();
    initAvailableInputs();
}

/**
 * @brief CompactRio::meanSpeed getter for the mean speed of the catamaran
 * @return mean speed [m/s]
 */
QPointF CompactRio::meanSpeed() const
{
    return m_meanSpeed;
}

/**
 * @brief CompactRio::crioSyncTimestamp getter for the remote (crio) timestamp
 * @return remote (crio) timestamp
 */
qint64 CompactRio::crioSyncTimestamp() const
{
    return m_crioTimestamp;
}

/**
 * @brief CompactRio::localSyncTimestamp getter for the local timestamp
 * @return local timestamp
 */
qint64 CompactRio::localSyncTimestamp() const
{
    return m_localTimestamp;
}

/**
 * @brief CompactRio::timesampSynchronized getter for the synchronization flag
 * @return true if timestamps (local and remote) are synchronized
 */
bool CompactRio::timesampSynchronized() const
{
    return m_currentTimeIsSet;
}

/**
 * @brief CompactRio::resetTimestampSynchronization clear the timestamp synchronization flag.
 *
 * This method is useful for exemple after a reconnection
 */
void CompactRio::resetTimestampSynchronization()
{
    m_currentTimeIsSet = false;
}

/**
 * @brief CompactRio::speed getter for the catamran instant speed
 * @return catamran speed [m/s]
 */
QPointF CompactRio::speed() const
{
    return m_speed;
}

/**
 * @brief CompactRio::position getter for the catamean position
 * @return catamran position [m]
 */
QPointF CompactRio::position() const
{
    return m_position;
}

/**
 * @brief CompactRio::heading getter for the catamran heading
 * @return catamran heading [°], 0° == North
 */
double CompactRio::heading() const
{
    return m_heading;
}

/**
 * @brief CompactRio::rightEngineValue getter for the right engine value
 * @return right engine value [-127:127]
 */
qint8 CompactRio::rightEngineValue() const
{
    return m_rightEngineValue;
}

/**
 * @brief CompactRio::leftEngineValue getter for the left engine value
 * @return left engine value [-127:127]
 */
qint8 CompactRio::leftEngineValue() const
{
    return m_leftEngineValue;
}

/**
 * @brief CompactRio::instance static method used to access singleton instance
 * @return the instance for this class
 */
CompactRio *CompactRio::instance()
{
    if(!s_instance){
        s_instance = new CompactRio();
    }
    return s_instance;
}

/**
 * @brief CompactRio::feedWithData method used to feed this object with data
 * This methods grab useful informations from the parameter data. Normaly this methode is used inside
 * the method MessageConsumer::handleDataMessage()
 * @param data reference on a CRioData used as input for feeding
 */
void CompactRio::feedWithData(const CRioData &data)
{
    bool valid;
    int addr = data.address.toInt(&valid);
    if(!valid){
        return;
    }
    switch(addr){
    case CRIO::DATA_ADDR_DEBUG:
        if(data.data().count() > 0){
            QString str = QDateTime::fromMSecsSinceEpoch(data.timestamp.unixTimestamp).toString("hh:mm:ss:zzz");
            str += ": [CRIO] ";
            str += data.data()[0].toString() + "\n";
            emit newCRioStatusMessage(str);
        }
        break;
    case CRIO::DATA_ADDR_NAV_SYS_LOG:
        if(data.data().count() > 0){
            QString str = QDateTime::fromMSecsSinceEpoch(data.timestamp.unixTimestamp).toString("hh:mm:ss:zzz");
            str += ": [ NS  ]";
            str += data.data()[0].toString() + "\n";
            emit newCRioStatusMessage(str);
        }
        break;
    case CRIO::DATA_ADDR_MOTOR_MODE:
        if(data.data().count() > 0){
            m_navMode = (CRIO::NAV_SYS_MODE) data.data()[0].value<quint8>();
            emit navigationModeChanged();
        }
        break;
    case CRIO::DATA_ADDR_LEFT_ENGINE_FEEDBACK:
        if(data.data().count() > 0){
            m_leftEngineValue = data.data()[0].value<qint8>();
            emit enginesChanged();
        }
        break;
    case CRIO::DATA_ADDR_RIGHT_ENGINE_FEEDBACK:
        if(data.data().count() > 0){
            m_rightEngineValue = data.data()[0].value<qint8>();
            emit enginesChanged();
        }
        break;
    case CRIO::DATA_ADDR_ORIENTATION:
        if(data.data().count() > 0){
            m_heading = data.data()[0].toDouble();
            emit headingChanged();
        }
        break;
    case CRIO::DATA_ADDR_POSITION:
        {
            qint64 tsDiff = data.timestamp.unixTimestamp - m_lastTimestamp.unixTimestamp;
            if (tsDiff > 3000) {
                emit nonConsecutiveDataReceived();
            }
            m_lastTimestamp = data.timestamp;
            if(data.data().count() > 1){
                m_position.setX(data.data()[0].toDouble());
                m_position.setY(data.data()[1].toDouble());
                emit positionChanged();
            }
            break;
        }
    case CRIO::DATA_ADDR_VELOCITY:
        if(data.data().count() > 1){
            m_speed.setX(data.data()[0].toDouble());
            m_speed.setY(data.data()[1].toDouble());
            m_pastSpeed.append(m_speed);
            processMeanSpeed();
            emit speedChanged();
        }
        break;
    case 62:
        if(data.data().count() > 1){
            double vx = data.data()[0].toDouble();
            double vy = data.data()[1].toDouble();
            double theta = data.data()[2].toDouble();
            qDebug() << "GPS course overground: vx="<<vx<<" vy="<<vy<<" v="<<sqrt(vx*vx+vy*vy)<<" theta="<<theta;
            emit speedChanged();
        }
        break;
    default:
        break;
    }
}

/**
 * @brief CompactRio::feedWithCommand method used to feed this object with command
 * This methods grab useful informations from the parameter cmd. Normaly this methode is used inside
 * the method MessageConsumer::on_dataReceived()
 * @param cmd reference on a CrioCommand object used for feeding
 */
void CompactRio::feedWithCommand(const CRioCommand &cmd)
{
    switch(cmd.command()){
    case CRIO::CMD_SET:
        switch(cmd.address()){
        case CRIO::CMD_ADDR_ENGINE_MODE:
        {
            if(cmd.parameters().count() < 2){ break;}
            CRIO::NAV_SYS_MODE nv = (CRIO::NAV_SYS_MODE) cmd.parameters()[1].toInt();
            if(nv != m_navMode){
                m_navMode = nv;
                emit navigationModeChanged();
            }
            break;
        }
        case CRIO::CMD_ADDR_LEFT_ENGINE:
        {
            if(cmd.parameters().count() < 2){ break;}
            qint8 v = cmd.parameters()[1].value<qint8>();
            if(v != m_leftEngineValue){
                m_leftEngineValue = v;
                emit enginesChanged();
            }
            break;
        }
        case CRIO::CMD_ADDR_RIGHT_ENGINE:
        {
            if(cmd.parameters().count() < 2){ break;}
            qint8 v = cmd.parameters()[1].value<qint8>();
            if(v != m_rightEngineValue){
                m_rightEngineValue = v;
                emit enginesChanged();
            }
            break;
        }
        case CRIO::CMD_ADDR_CURRENT_TIME:
        {
            if(cmd.parameters().count() < 2){ break;}
            CRIO::Timestamp ts = cmd.parameters()[1].value<CRIO::Timestamp>();
            if(!m_currentTimeIsSet || ts.unixTimestamp != m_crioTimestamp){
                m_currentTimeIsSet = true;
                m_crioTimestamp = ts.timestamp;
                m_localTimestamp = QDateTime::currentMSecsSinceEpoch();

                qint64 delta = m_localTimestamp - m_crioTimestamp * 1000;
                CRIO::Timestamp::timestampDeltaMs = delta;
                emit syncTimestampChanged();
            }
            break;
        }
        case CRIO::CMD_ADDR_HONK:

            break;
        case CRIO::CMD_ADDR_LIGHT:

            break;
        case CRIO::CMD_ADDR_MEMORY_16BIT:

            break;
        case CRIO::CMD_ADDR_PRISME_TS_SYNC:

            break;
        case CRIO::CMD_ADDR_NS_CSTS:

            break;
        case CRIO::CMD_ADDR_NS_LIMITS:

            break;
        case CRIO::CMD_ADDR_NS_WAYPOINTS:

            break;
        default:
            break;
        }
        break;
    case CRIO::CMD_NOTIFY:
        switch(cmd.address()){
        case CRIO::NOTIFY_NS_NEW_LINE:
            notificationNavSysNewLinePointIndexes(cmd.parameters()[2].value<quint16>(), cmd.parameters()[3].value<quint16>(), cmd.parameters()[2].value<CRIO::Timestamp>());
            break;
        case CRIO::NOTIFY_NS_WP_REACHED:
            notificationNavSysWaypointReached(cmd.parameters()[2].value<quint16>(), cmd.parameters()[2].value<CRIO::Timestamp>());
        default:
            break;
        }

    default:
        break;
    }
}

/**
 * @brief CompactRio::waypoints getter for waypoints list
 * @return waypoints
 */
const QList<QPointF> &CompactRio::waypoints() const
{
    return m_waypoints;
}

/**
 * @brief CompactRio::waypoint getter for a single waypoint at index "index"
 * @param index index of the desired waypoint
 * @return if index is valid return the desired waypoint otherwise return a default QPoinF
 */
QPointF CompactRio::waypoint(int index) const
{
    if(index < m_waypoints.count()){
        return m_waypoints.at(index);
    }
    return QPointF();
}

/**
 * @brief CompactRio::addWaypoint append a waypoint to the waypoints list and return the new size of the list
 * @param wp the waypoint to add
 * @return size of the number of waypoints in the list
 */
int CompactRio::addWaypoint(const QPointF &wp)
{
    m_waypoints.append(wp);
    return m_waypoints.count();
}

/**
 * @brief CompactRio::removeWaypoint remove the waypoint pointed by index from  the waypoints list
 * @param index
 * @return true on success, false otherwise
 */
bool CompactRio::removeWaypoint(int index)
{
    if(index < m_waypoints.count()){
        m_waypoints.removeAt(index);
        return true;
    }
    return false;
}

/**
 * @brief CompactRio::removeLastWaypoint remove the last waypoint in the waypoints list
 */
void CompactRio::removeLastWaypoint()
{
    if(!m_waypoints.isEmpty()){
        m_waypoints.pop_front();
    }
}

/**
 * @brief CompactRio::clearWaypoint clear the waypoints list
 * @param force
 */
void CompactRio::clearWaypoint(bool force)
{
    Q_UNUSED(force);
    m_waypoints.clear();
}

/**
 * @brief CompactRio::availableInputs getter for the list off available inputs
 * @return the list of available inputs
 */
const QList<CompactRio::NamedAddress> CompactRio::availableInputs() const
{
    return m_availableInputs;
}

/**
 * @brief CompactRio::selfAllocatedSensors getter for all the immutable sensors needed by the catamran
 * @return the list of all self allocated sensors
 */
const QList<Sensor *> CompactRio::selfAllocatedSensors() const
{
    return m_selfAllocatedSensors;
}

void CompactRio::processMeanSpeed()
{
    m_meanSpeed = QPointF(0,0);
    if(m_pastSpeed.isEmpty()){
        return;
    }
    while(m_pastSpeed.count() > 10){
        m_pastSpeed.removeFirst();
    }
    int n=0;
    foreach(QPointF p, m_pastSpeed){
        m_meanSpeed += p;
        ++n;
    }
    m_meanSpeed /= n;
}

/**
 * @brief CompactRio::initSelftAllocatedSensors initialize the compactRio object
 * The method create all the self allocated sensors and add them to the list
 */
void CompactRio::initSelftAllocatedSensors()
{
    SensorTypeManager::instance()->createType("GPS Position");
    SensorTypeManager::instance()->createType("GPS Speed");
    SensorTypeManager::instance()->createType("Compass");
    SensorTypeManager::instance()->createType("Temperature");
    SensorTypeManager::instance()->createType("Wind Speed");
    SensorTypeManager::instance()->createType("Wind Direction");
    SensorTypeManager::instance()->createType("Radiometer");
    SensorTypeManager::instance()->createType("ADCP");
    Sensor *gpsPos = new Sensor("48", "GPS[position]", SensorTypeManager::instance()->type("GPS Position"), true, true, "GPS", "", false);
    Sensor *gpsSpeed = new Sensor("49", "GPS[velocity]", SensorTypeManager::instance()->type("GPS Speed"), true, true, "GPS", "", false);
    Sensor *compass = new Sensor("41", "Compass[heading]", SensorTypeManager::instance()->type("Compass"), true, true, "Compass", "", false);
    Sensor *rightEngine = new Sensor("51", "Right engine", SensorTypeManager::instance()->type("Engines"), true, true, "Engines", "", false);
    Sensor *leftEngine = new Sensor("52",  "Left engine", 0, true, true, "Engines", "", false);
    Sensor *crioDebug = new Sensor("60", "crioDebug", 0, false, true, "", "", false);
    Sensor *navSysLog = new Sensor("61", "NavSysLog", 0, false, true, "", "", false);

    Sensor *gpsSpeed2 = new Sensor("62", "GPS[velocity2]", SensorTypeManager::instance()->type("GPS Speed"), true, true, "GPS", "", false);

    TransformationBaseClass *prismeTransformation = new PRisme();
    TransformationManager::instance()->addTransformation(prismeTransformation);
    //Sensor *prisme = new Sensor("65", "PRisme", SensorTypeManager::instance()->type("Unknown"), 0, false, true, "", "", false);
    //prisme->setTransformation(prismeTransformation);

    m_selfAllocatedSensors.append(gpsPos);
    m_selfAllocatedSensors.append(gpsSpeed);
    m_selfAllocatedSensors.append(compass);
    m_selfAllocatedSensors.append(leftEngine);
    m_selfAllocatedSensors.append(rightEngine);
    m_selfAllocatedSensors.append(crioDebug);
    m_selfAllocatedSensors.append(navSysLog);

    m_selfAllocatedSensors.append(gpsSpeed2);
    //m_selfAllocatedSensors.append(prisme);
}

/**
 * @brief CompactRio::initAvailableInputs getter for all the available inputs
 */
void CompactRio::initAvailableInputs()
{
    m_availableInputs.append(NamedAddress(64 , "Serial M0P0"));
    m_availableInputs.append(NamedAddress(65 , "Serial M0P1"));
    m_availableInputs.append(NamedAddress(66 , "Serial M0P2"));
    m_availableInputs.append(NamedAddress(67 , "Serial M0P3"));

    m_availableInputs.append(NamedAddress(96 , "PT100 M0P0"));
    m_availableInputs.append(NamedAddress(97 , "PT100 M0P1"));
    m_availableInputs.append(NamedAddress(98 , "PT100 M0P2"));
    m_availableInputs.append(NamedAddress(99 , "PT100 M0P3"));
    m_availableInputs.append(NamedAddress(100, "PT100 M1P0"));
    m_availableInputs.append(NamedAddress(101, "PT100 M1P1"));
    m_availableInputs.append(NamedAddress(102, "PT100 M1P2"));
    m_availableInputs.append(NamedAddress(103, "PT100 M1P3"));

    m_availableInputs.append(NamedAddress(53, "Impulse counter"));
}

/**
 * @brief CompactRio::setEngine set the engine value on the catamran
 * This tool build and send a "setEngine" command to the catamran
 * @param engine desired engine (CRIO::LEFT / CRIO::RIGHT)
 * @param value the value for the engine [-127:127]
 * @return true on success, false otherwise
 */
bool CompactRio::setEngine(const CRIO::Engines engine, const qint8 value){
    CRioCommand cmd(CRIO::CMD_SET, (engine==CRIO::LEFT)?CRIO::CMD_ADDR_LEFT_ENGINE:CRIO::CMD_ADDR_RIGHT_ENGINE);
    cmd.addParameter(QVariant::fromValue(value));
    return m_server->sendMessage(cmd);
}

bool CompactRio::stop(){
    CRioCommand cmd(CRIO::CMD_STOP);
    return m_server->sendMessage(cmd);
}

bool CompactRio::addWaypointCmd(const QPointF &p, int index){
    CRioCommand cmd(CRIO::CMD_ADD, CRIO::CMD_ADDR_NS_WAYPOINTS);
    cmd.addParameter(QVariant(p));
    if(index >= 0){
        cmd.addParameter(QVariant::fromValue((quint16) index));
    }
    return m_server->sendMessage(cmd);
}

bool CompactRio::setWaypointsCmd(const QList<QPointF> &points){
    CRioCommand cmd(CRIO::CMD_ADD, CRIO::CMD_ADDR_NS_WAYPOINTS);
    foreach(QPointF p, points){
        cmd.addParameter(QVariant(p));
    }
    return m_server->sendMessage(cmd);
}

bool CompactRio::setSensorsConfig()
{
    CRioCommand cmd(CRIO::CMD_SET, CRIO::CMD_ADDR_SENSOR_CONFIG);
    cmd.addParameter(QVariant::fromValue(SensorConfig::instance()->getSensors()));
    return m_server->sendMessage(cmd);
}

bool CompactRio::setNavSysMode(const CRIO::NAV_SYS_MODE &mode)
{
    CRioCommand cmd(CRIO::CMD_SET, CRIO::CMD_ADDR_ENGINE_MODE);
    cmd.addParameter(QVariant::fromValue((quint8) mode));
    return m_server->sendMessage(cmd);
}


bool CompactRio::setNavSysConstants(const double C_perp, const double C_point, const double C_aheadD, const double Kp_Y, const double Kp_V)
{
    CRioCommand cmd(CRIO::CMD_SET, CRIO::CMD_ADDR_NS_CSTS);
    cmd.addParameter(QVariant::fromValue(C_perp));
    cmd.addParameter(QVariant::fromValue(C_point));
    cmd.addParameter(QVariant::fromValue(C_aheadD));
    cmd.addParameter(QVariant::fromValue(Kp_Y));
    cmd.addParameter(QVariant::fromValue(Kp_V));
    return m_server->sendMessage(cmd);
}

bool CompactRio::setNavSysLimits(const double delta, const double epsilon)
{
    CRioCommand cmd(CRIO::CMD_SET, CRIO::CMD_ADDR_NS_LIMITS);
    cmd.addParameter(QVariant::fromValue(epsilon));
    cmd.addParameter(QVariant::fromValue(delta));
    return m_server->sendMessage(cmd);
}

bool CompactRio::setNavSysSpeedSetpoint(const double setpoint)
{
    CRioCommand cmd(CRIO::CMD_SET, CRIO::CMD_ADDR_NS_SPEED_SETPOINT);
    cmd.addParameter(QVariant::fromValue(setpoint));
    return m_server->sendMessage(cmd);
}

bool CompactRio::setHonk(const CRIO::ON_OFF &mode)
{
    CRioCommand cmd(CRIO::CMD_SET, CRIO::CMD_ADDR_HONK);
    cmd.addParameter(QVariant::fromValue(mode == CRIO::ON));
    return m_server->sendMessage(cmd);
}

bool CompactRio::setLight(const CRIO::ON_OFF &mode)
{
    CRioCommand cmd(CRIO::CMD_SET, CRIO::CMD_ADDR_LIGHT);
    cmd.addParameter(QVariant::fromValue(mode == CRIO::ON));
    return m_server->sendMessage(cmd);
}

bool CompactRio::setSabertoothState(const CRIO::ON_OFF &mode)
{
    CRioCommand cmd(CRIO::CMD_SET, CRIO::CMD_ADDR_SABERTOOTH_ENABLE);
    cmd.addParameter(QVariant::fromValue(mode == CRIO::ON));
    return m_server->sendMessage(cmd);
}

bool CompactRio::setSabertoothConfig(const quint8 &configAddr, const quint8 &value)
{
    CRioCommand cmd(CRIO::CMD_SET, CRIO::CMD_ADDR_LIGHT);
    cmd.addParameter(QVariant::fromValue(configAddr));
    cmd.addParameter(QVariant::fromValue(value));
    return m_server->sendMessage(cmd);
}

bool CompactRio::getCommand(const CRIO::CommandAddresses &addr)
{
    CRioCommand cmd(CRIO::CMD_GET, addr);
    return m_server->sendMessage(cmd);
}

bool CompactRio::getCurrentTime()
{
    CRioCommand cmd(CRIO::CMD_GET, CRIO::CMD_ADDR_CURRENT_TIME);
    return m_server->sendMessage(cmd);
}

bool CompactRio::get16bMemory(const CRIO::Memory::CMD_ADDR_16B_BLOC &addr)
{
    CRioCommand cmd(CRIO::CMD_GET, CRIO::CMD_ADDR_MEMORY_16BIT);
    cmd.addParameter(QVariant::fromValue((quint8) addr));
    return m_server->sendMessage(cmd);
}

bool CompactRio::set16bMemory(const CRIO::Memory::CMD_ADDR_16B_BLOC &addr, const quint16 &value)
{
    CRioCommand cmd(CRIO::CMD_SET, CRIO::CMD_ADDR_MEMORY_16BIT);
    cmd.addParameter(QVariant::fromValue((quint8) addr));
    cmd.addParameter(QVariant::fromValue(value));
    return m_server->sendMessage(cmd);
}

bool CompactRio::setPRismeSamplingRate(quint8 n100ms, quint8 portAddress)
{
    QVariantList d;
    QString bytes;
    bytes.append('s');
    bytes.append((char) portAddress);
    bytes.append((char) n100ms);
    d.append(bytes);
    CRioData data(CRIO::DATA_ADDR_NI9870_2_P1, d, CRIO::Timestamp());
    return m_server->sendMessage(data);
}

bool CompactRio::getPRismeSyncTimestamp()
{
    CRioCommand cmd(CRIO::CMD_GET, CRIO::CMD_ADDR_PRISME_TS_SYNC);
    return m_server->sendMessage(cmd);
}

bool CompactRio::setFpgaCounterSamplingTime(const quint16 &ms)
{
    CRioCommand cmd(CRIO::CMD_SET, CRIO::CMD_ADDR_MEMORY_16BIT);
    cmd.addParameter(QVariant::fromValue((quint8) CRIO::Memory::FPGA_COUNTER_1_SAMPLING_TIME));
    cmd.addParameter(QVariant::fromValue(ms));
    return m_server->sendMessage(cmd);
}

void CompactRio::notificationNavSysNewLinePointIndexes(quint16 id1, quint16 id2, const CRIO::Timestamp &ts)
{
    qDebug() << "WP new line used id1=" << id1 << " id2="<<id2 << " ts=" << ts.unixTimestamp;
    m_waypointsUsedInLineIndexes.first = id1;
    m_waypointsUsedInLineIndexes.second = id2;
    emit navSysNewLineInUse();
}

void CompactRio::notificationNavSysWaypointReached(quint16 id2, const CRIO::Timestamp &ts)
{
    qDebug() << "WP reached id=" << id2 << " ts=" << ts.unixTimestamp;
    emit navSysWaypointReached(id2);
}

bool CompactRio::delWaypointCmd()
{
    CRioCommand cmd(CRIO::CMD_DEL, CRIO::CMD_ADDR_NS_WAYPOINTS);
    return m_server->sendMessage(cmd);
}
