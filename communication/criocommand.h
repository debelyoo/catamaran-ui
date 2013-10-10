#ifndef CRIOCOMMAND_H
#define CRIOCOMMAND_H

#include "util/criodatastream.h"
#include "util/criobytearray.h"
#include "util/criodefinitions.h"
#include "model/sensor.h"

class CRioCommand : public CRIO::Object
{
public:
    CRioCommand(CRIO::CommandTypes cmd, CRIO::CommandAddresses address, const QList<QVariant> &parameters);
    CRioCommand(CRIO::CommandTypes cmd, CRIO::CommandAddresses address = CRIO::ADDR_NO_ADDRESS);
    static Object *create(CRioDataStream &ds);

    CRIO::CommandTypes command() const;
    CRIO::CommandAddresses address() const;
    QList<QVariant> parameters() const;
    void addParameter(QVariant p);
protected:
    CRIO::CommandTypes m_command;
    QList<QVariant> m_parameters;
    CRIO::CommandAddresses m_address;
};

class CRioByteArray;
namespace CRIO {
    CRioByteArray setEngine(const Engines engine, const qint8 value);
    CRioByteArray stop();

    CRioByteArray setSensorsConfig(const QList<Sensor *> &sensors);
    CRioByteArray setNavSysMode(const CRIO::NAV_SYS_MODE &mode);
    CRioByteArray setNavSysConstants(const double C_perp, const double C_point, const double C_aheadD, const double Kp_Y, const double Kp_V);
    CRioByteArray setNavSysLimits(const double delta, const double epsilon);
    CRioByteArray setHonk(const CRIO::ON_OFF &mode);
    CRioByteArray setLight(const CRIO::ON_OFF &mode);
    CRioByteArray setSabertoothState(const CRIO::ON_OFF &mode);
    CRioByteArray setSabertoothConfig(const quint8 &configAddr, const quint8 &value);

    CRioByteArray addWaypointCmd(const QPointF &p, int index = -1);
    CRioByteArray setWaypointsCmd(const QList<QPointF> &points);
    CRioByteArray delWaypointCmd();

    CRioByteArray getCommand(const CRIO::CommandAddresses &addr);

    // Memories
    CRioByteArray get16bMemory(const CRIO::Memory::ADDR_16B_BLOC &addr);
    CRioByteArray set16bMemory(const CRIO::Memory::ADDR_16B_BLOC &addr, const quint16 &value);

    // PT100 config

    // FGPA counter config
    CRioByteArray setFpgaCounterSamplingTime(const quint16 &ms);
}



#endif // CRIOCOMMAND_H
