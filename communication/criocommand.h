#ifndef CRIOCOMMAND_H
#define CRIOCOMMAND_H

#include "util/criobytearray.h"
#include "util/criodefinitions.h"

class Sensor;

/**
 * @brief Commands used to interact with the catamaran
 */
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
    CRIO::CommandTypes m_command;           /// Command type [GET, SET, STOP, ADD, REMOVE, NOTIFY]
    QList<QVariant> m_parameters;           /// Parameters of the command
    CRIO::CommandAddresses m_address;       /// Specific parameter (1st parameter) used for GET/SET/NOTIFY/REMOVE/ADD commands
};

#endif // CRIOCOMMAND_H
