#include "compactrio.h"
#include "manager/sensortypemanager.h"

CompactRio *CompactRio::s_instance = NULL;

CompactRio::CompactRio():
    QObject(),
    AbstractCrioStatesHolder(),
    m_server(Server::instance()),
    m_sensorConfig(SensorConfig::instance()),
    m_syncTimestamp(),
    m_navMode(),
    m_leftEngineValue(),
    m_rightEngineValue(),
    m_heading(),
    m_position(),
    m_speed(),
    m_meanSpeed(),
    m_pastSpeed(),
    m_selfAllocatedSensors()
{
    initSelftAllocatedSensors();
    initAvailableInputs();
}
QPointF CompactRio::meanSpeed() const
{
    return m_meanSpeed;
}

QPointF CompactRio::speed() const
{
    return m_speed;
}

QPointF CompactRio::position() const
{
    return m_position;
}

double CompactRio::heading() const
{
    return m_heading;
}

qint8 CompactRio::rightEngineValue() const
{
    return m_rightEngineValue;
}

qint8 CompactRio::leftEngineValue() const
{
    return m_leftEngineValue;
}

CompactRio *CompactRio::instance()
{
    if(!s_instance){
        s_instance = new CompactRio();
    }
    return s_instance;
}

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
            str += ": [CRIO]";
            str += data.data()[0].toString();
            emit newCRioStatusMessage(str);
        }
        break;
    case CRIO::DATA_ADDR_NAV_SYS_LOG:
        if(data.data().count() > 0){
            QString str = QDateTime::fromMSecsSinceEpoch(data.timestamp.unixTimestamp).toString("hh:mm:ss:zzz");
            str += ": [ NS ]";
            str += data.data()[0].toString();
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
        if(data.data().count() > 1){
            m_position.setX(data.data()[0].toDouble());
            m_position.setY(data.data()[1].toDouble());
            emit positionChanged();
        }
        break;
    case CRIO::DATA_ADDR_VELOCITY:
        if(data.data().count() > 1){
            m_speed.setX(data.data()[0].toDouble());
            m_speed.setY(data.data()[1].toDouble());
            m_pastSpeed.append(m_speed);
            processMeanSpeed();
            emit speedChanged();
        }
        break;
    default:
        break;
    }
}

void CompactRio::feedWithCommand(const CRioCommand &cmd)
{

}

const QList<CompactRio::NamedAddress> CompactRio::availableInputs() const
{
    return m_availableInputs;
}

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

void CompactRio::initSelftAllocatedSensors()
{
    SensorTypeManager::instance()->createType("GPS Position", "", 0);
    SensorTypeManager::instance()->createType("GPS Speed", "", 0);
    SensorTypeManager::instance()->createType("Compass", "", 0);
    Sensor *gpsPos = new Sensor("48", "GPS[position]", SensorTypeManager::instance()->type("GPS Position"), 0, true, true, "GPS");
    Sensor *gpsSpeed = new Sensor("49", "GPS[velocity]", SensorTypeManager::instance()->type("GPS Position"), 0, true, true, "GPS");
    Sensor *compass = new Sensor("41", "Compass[heading]", SensorTypeManager::instance()->type("Compass"), 0, true, true, "Compass");
    m_selfAllocatedSensors.append(gpsPos);
    m_selfAllocatedSensors.append(gpsSpeed);
    m_selfAllocatedSensors.append(compass);
}

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
}

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

bool CompactRio::setSensorsConfig(const QList<Sensor *> &sensors)
{
    CRioCommand cmd(CRIO::CMD_SET, CRIO::CMD_ADDR_SENSOR_CONFIG);
    cmd.addParameter(QVariant::fromValue(sensors));
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

bool CompactRio::setFpgaCounterSamplingTime(const quint16 &ms)
{
    CRioCommand cmd(CRIO::CMD_SET, CRIO::CMD_ADDR_MEMORY_16BIT);
    cmd.addParameter(QVariant::fromValue((quint8) CRIO::Memory::FPGA_COUNTER_1_SAMPLING_TIME));
    cmd.addParameter(QVariant::fromValue(ms));
    return m_server->sendMessage(cmd);
}

bool CompactRio::delWaypointCmd()
{
    CRioCommand cmd(CRIO::CMD_DEL, CRIO::CMD_ADDR_NS_WAYPOINTS);
    return m_server->sendMessage(cmd);
}
