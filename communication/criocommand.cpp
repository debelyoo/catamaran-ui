#include "criocommand.h"

#include <QDebug>
#include "model/sensor.h"
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
    CRIO::Object(),
    m_command(cmd),
    m_parameters(),
    m_address(address)
{
}

/**
 * @brief CRioCommand::command getter for command type
 * @return command type
 */
CRIO::CommandTypes CRioCommand::command() const
{
    return m_command;
}

/**
 * @brief CRioCommand::address getter for the address
 * @return address
 */
CRIO::CommandAddresses CRioCommand::address() const
{
    return m_address;
}

/**
 * @brief CRioCommand::parameters getter for parameters
 *
 * For GET/SET/REMOVE/ADD/NOTIFY commands, the address is added in front of the parameters list.
 * @return
 */
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

/**
 * @brief CRioCommand::addParameter add a parameter to the parameters list
 * @param p the parameter
 */
void CRioCommand::addParameter(QVariant p){
    m_parameters.append(p);
}
