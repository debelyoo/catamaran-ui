#include "registeredsensoritem.h"

RegisteredSensorItem::RegisteredSensorItem(SensorInputItem *sensorInput, RegisteredSensorItem *parent):
    m_parent(parent),
    m_name(sensorInput->fullName()),
    m_transformation(NULL),
    m_sortId(QString::number(sensorInput->sortId()))
{
}

RegisteredSensorItem *RegisteredSensorItem::parent() const
{
    return m_parent;
}

void RegisteredSensorItem::setParent(RegisteredSensorItem *parent)
{
    m_parent = parent;
}
QVector<RegisteredSensorItem *> RegisteredSensorItem::childs() const
{
    return m_childs;
}

void RegisteredSensorItem::setChilds(const QVector<RegisteredSensorItem *> &childs)
{
    m_childs = childs;
}

void RegisteredSensorItem::addChild(RegisteredSensorItem *child)
{
    m_childs.append(child);
    child->setParent(this);
}

void RegisteredSensorItem::insertChild(RegisteredSensorItem *child, int pos)
{
    m_childs.insert(pos, child);
}

RegisteredSensorItem *RegisteredSensorItem::removeChild(RegisteredSensorItem *child)
{
    int index = -1;
    for(int i=0;i<m_childs.count(); ++i){
        if(m_childs[i] == child){
            index = i;
            break;
        }
    }
    if(index >= 0){
        RegisteredSensorItem *c = m_childs[index];
        m_childs.remove(index);
        return c;
    }
    return NULL;
}

RegisteredSensorItem *RegisteredSensorItem::child(int index) const
{
    if(index >= m_childs.count()){
        return NULL;
    }
    return m_childs[index];
}

QString RegisteredSensorItem::name() const
{
    return m_name;
}

void RegisteredSensorItem::setName(QString name)
{
    m_name = name;
}

int RegisteredSensorItem::childCount() const
{
    return m_childs.count();
}

int RegisteredSensorItem::childNumber() const
{
    if(m_parent){
        return m_parent->childs().indexOf(const_cast<RegisteredSensorItem *>(this));

    }
    return 0;
}

QVariant RegisteredSensorItem::data(int col) const
{
    switch(col){
    case 0:
        return m_name;
    case 1:
        if(m_transformation){
            return QVariant(m_transformation->getTransformationDefinition().name);
        }
        return QVariant("None");
    case 2:
        return QVariant();
    default:
        return QVariant();
    }
}

QString RegisteredSensorItem::sortId()
{
    return m_sortId;
}




