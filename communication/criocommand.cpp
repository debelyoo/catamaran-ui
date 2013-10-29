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
    m_command(cmd),
    m_parameters(),
    m_address(address)
{
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
