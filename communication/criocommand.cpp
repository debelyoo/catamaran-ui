#include "criocommand.h"
#include <QDebug>

/*
 *  Command class definition
 */
//CRioCommand::Command(): m_command(CRIO::CMD_GET), m_parameters(){}
CRioCommand::CRioCommand(CRIO::CommandTypes cmd, CRIO::CommandAddresses address, const QList<QVariant> &parameters):
    CRIO::Object(),
    m_command(cmd),
    m_parameters(parameters),
    m_address(address)
{
}

CRioCommand::CRioCommand(CRIO::CommandTypes cmd, CRIO::CommandAddresses address):
    m_command(cmd),
    m_parameters(),
    m_address(address)
{
}

CRIO::Object *CRioCommand::create(CRioDataStream &ds)
{
    bool error = false;
    quint8 cmdType;
    ds >> cmdType;
    quint32 nParams;
    ds >> nParams;
    QVariantList params;
    quint8 address = 0;
    switch(cmdType){
    case CRIO::CMD_GET:
    {
        quint32 strlen;
        ds >> strlen;
        ds >> address;
    }
        break;
    case CRIO::CMD_SET:
    {
        quint32 strlen;
        if(nParams > 0){
            ds >> strlen;
            ds >> address;
            --nParams;
        }
        switch(address){
        case CRIO::ADDR_PRISME_TS_SYNC:
        {
            ds >> strlen;
            qint64 secs;
            quint64 fracs;
            ds >> secs;
            ds >> fracs;
            CRIO::Timestamp ts(secs, fracs);
            params.append(QVariant::fromValue(ts.timestamp));
        }
            break;
        default:
            break;
        }
    }
        break;
    case CRIO::CMD_ADD:
    {
        //ds >> address;
    }
        break;
    case CRIO::CMD_DEL:
    {
        //ds >> address;
    }
        break;
    case CRIO::CMD_STOP:
        break;
    }

    if(error){
        return NULL;
    }
    return new CRioCommand((CRIO::CommandTypes)cmdType, (CRIO::CommandAddresses)address, params);
}

CRIO::CommandTypes CRioCommand::command() const
{
    return m_command;
}

CRIO::CommandAddresses CRioCommand::address() const
{
    return m_address;
}

QList<QVariant> CRioCommand::parameters() const
{
    if(m_command == CRIO::CMD_STOP){
        return m_parameters;
    }
    QVariantList params;
    params.append(QVariant::fromValue((quint8) m_address));
    params.append(m_parameters);
    return params;
}

void CRioCommand::addParameter(QVariant p){
    m_parameters.append(p);
}

/*
 *  Command factory method
 */
CRioByteArray CRIO::setEngine(const Engines engine, const qint8 value){
    CRioCommand cmd(CMD_SET, (engine==LEFT)?ADDR_LEFT_ENGINE:ADDR_RIGHT_ENGINE);
    cmd.addParameter(QVariant::fromValue(value));
    qDebug() << "Send Engine Cmd";
    return CRioByteArray(cmd);
}

CRioByteArray CRIO::stop(){
    CRioCommand cmd(CMD_STOP);
    qDebug() << "Send STOP Cmd";
    return CRioByteArray(cmd);
}

CRioByteArray CRIO::addWaypointCmd(const QPointF &p, int index){
    CRioCommand cmd(CMD_ADD, ADDR_NS_WAYPOINTS);
    cmd.addParameter(QVariant(p));
    if(index >= 0){
        cmd.addParameter(QVariant::fromValue((quint16) index));
    }
    qDebug() << "Send addWayPoint Cmd";
    return CRioByteArray(cmd);
}

CRioByteArray CRIO::setWaypointsCmd(const QList<QPointF> &points){
    CRioCommand cmd(CMD_ADD, ADDR_NS_WAYPOINTS);
    foreach(QPointF p, points){
        cmd.addParameter(QVariant(p));
    }
    qDebug() << "Send setWaypoints Cmd";
    return CRioByteArray(cmd);
}

CRioByteArray CRIO::setSensorsConfig(const QList<Sensor *> &sensors)
{
    CRioCommand cmd(CMD_SET, ADDR_SENSOR_CONFIG);
    cmd.addParameter(QVariant::fromValue(sensors));
    qDebug() << "Send SensorConfig Cmd";
    return CRioByteArray(cmd);
}

CRioByteArray CRIO::setNavSysMode(const CRIO::NAV_SYS_MODE &mode)
{
    CRioCommand cmd(CMD_SET, ADDR_ENGINE_MODE);
    cmd.addParameter(QVariant::fromValue((quint8) mode));
    qDebug() << "Send Nav mode Cmd";
    return CRioByteArray(cmd);
}


CRioByteArray CRIO::setNavSysConstants(const double C_perp, const double C_point, const double C_aheadD, const double Kp_Y, const double Kp_V)
{
    CRioCommand cmd(CMD_SET, ADDR_NS_CSTS);
    cmd.addParameter(QVariant::fromValue(C_perp));
    cmd.addParameter(QVariant::fromValue(C_point));
    cmd.addParameter(QVariant::fromValue(C_aheadD));
    cmd.addParameter(QVariant::fromValue(Kp_Y));
    cmd.addParameter(QVariant::fromValue(Kp_V));
    return CRioByteArray(cmd);
}


CRioByteArray CRIO::setNavSysLimits(const double delta, const double epsilon)
{
    CRioCommand cmd(CMD_SET, ADDR_NS_LIMITS);
    cmd.addParameter(QVariant::fromValue(epsilon));
    cmd.addParameter(QVariant::fromValue(delta));
    return CRioByteArray(cmd);
}


CRioByteArray CRIO::setHonk(const CRIO::ON_OFF &mode)
{
    CRioCommand cmd(CMD_SET, ADDR_HONK);
    cmd.addParameter(QVariant::fromValue(mode == CRIO::ON));
    return CRioByteArray(cmd);
}


CRioByteArray CRIO::setLight(const CRIO::ON_OFF &mode)
{
    CRioCommand cmd(CMD_SET, ADDR_LIGHT);
    cmd.addParameter(QVariant::fromValue(mode == CRIO::ON));
    return CRioByteArray(cmd);
}


CRioByteArray CRIO::setSabertoothState(const CRIO::ON_OFF &mode)
{
    CRioCommand cmd(CMD_SET, ADDR_SABERTOOTH_ENABLE);
    cmd.addParameter(QVariant::fromValue(mode == CRIO::ON));
    return CRioByteArray(cmd);
}


CRioByteArray CRIO::setSabertoothConfig(const quint8 &configAddr, const quint8 &value)
{
    CRioCommand cmd(CMD_SET, ADDR_LIGHT);
    cmd.addParameter(QVariant::fromValue(configAddr));
    cmd.addParameter(QVariant::fromValue(value));
    return CRioByteArray(cmd);
}


CRioByteArray CRIO::getCommand(const CRIO::CommandAddresses &addr)
{
    CRioCommand cmd(CMD_GET, addr);
    return CRioByteArray(cmd);
}


CRioByteArray CRIO::get16bMemory(const CRIO::Memory::ADDR_16B_BLOC &addr)
{
    CRioCommand cmd(CMD_GET, CRIO::ADDR_MEMORY_16BIT);
    cmd.addParameter(QVariant::fromValue((quint8) addr));
    return CRioByteArray(cmd);
}


CRioByteArray CRIO::set16bMemory(const CRIO::Memory::ADDR_16B_BLOC &addr, const quint16 &value)
{
    CRioCommand cmd(CMD_SET, CRIO::ADDR_MEMORY_16BIT);
    cmd.addParameter(QVariant::fromValue((quint8) addr));
    cmd.addParameter(QVariant::fromValue(value));
    return CRioByteArray(cmd);
}


CRioByteArray CRIO::setFpgaCounterSamplingTime(const quint16 &ms)
{
    CRioCommand cmd(CMD_SET, CRIO::ADDR_MEMORY_16BIT);
    cmd.addParameter(QVariant::fromValue((quint8) CRIO::Memory::FPGA_COUNTER_1_SAMPLING_TIME));
    cmd.addParameter(QVariant::fromValue(ms));
    return CRioByteArray(cmd);
}


CRioByteArray CRIO::delWaypointCmd()
{
    CRioCommand cmd(CMD_DEL, CRIO::ADDR_NS_WAYPOINTS);
    return CRioByteArray(cmd);
}
