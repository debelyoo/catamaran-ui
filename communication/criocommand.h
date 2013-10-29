#ifndef CRIOCOMMAND_H
#define CRIOCOMMAND_H

#include "util/criobytearray.h"
#include "util/criodefinitions.h"

class Sensor;
class CRioCommand : public CRIO::Object
{
public:
    CRioCommand(CRIO::CommandTypes cmd, CRIO::CommandAddresses address, const QList<QVariant> &parameters);
    CRioCommand(CRIO::CommandTypes cmd, CRIO::CommandAddresses address = CRIO::CMD_ADDR_NO_ADDRESS);

    CRIO::CommandTypes command() const;
    CRIO::CommandAddresses address() const;
    QList<QVariant> parameters() const;
    void addParameter(QVariant p);
protected:
    CRIO::CommandTypes m_command;
    QList<QVariant> m_parameters;
    CRIO::CommandAddresses m_address;
};

#endif // CRIOCOMMAND_H
