#include "sensorinputitem.h"
#include <QDebug>

SensorInputItem::SensorInputItem(const QString &address, const QString &name, SensorInputItem *pParent):
    QObject(),
    m_pParent(pParent),
    m_name(name),
    m_address(address),
    m_enabled(true)
{
}

SensorInputItem::SensorInputItem(const QString &name, SensorInputItem *pParent):
    QObject(),
    m_pParent(pParent),
    m_name(name),
    m_address(name),
    m_enabled(true)
{
}

SensorInputItem::~SensorInputItem()
{
    if(m_pParent){
        m_pParent->removeChild(this->childNumber());
    }
}

SensorInputItem *SensorInputItem::parent() const
{
    return m_pParent;
}

void SensorInputItem::setParent(SensorInputItem *pParent)
{
    m_pParent = pParent;
}
QList<SensorInputItem *> SensorInputItem::childs() const
{
    return m_childs;
}

void SensorInputItem::setChilds(const QList<SensorInputItem *> &childs)
{
    m_childs = childs;
}

void SensorInputItem::addChild(SensorInputItem *child)
{
    m_childs.append(child);
    child->setParent(this);
    emit itemChanged();
}

void SensorInputItem::insertChild(SensorInputItem *child, int pos)
{
    m_childs.insert(pos, child);
}

bool SensorInputItem::removeChild(int index)
{
    if(index < m_childs.count() && index >= 0){
        SensorInputItem *c = m_childs[index];
        if(c){
            m_childs.removeAt(index);
            emit itemChanged();
            return true;
        }
    }

    return false;
}

void SensorInputItem::deleteChilds()
{
    foreach(SensorInputItem *i, m_childs){
        m_childs.removeOne(i);
        emit itemChanged();
        delete i;
    }

}

SensorInputItem *SensorInputItem::child(int index) const
{
    if(index >= m_childs.count()){
        return NULL;
    }
    return m_childs[index];
}

QString SensorInputItem::name() const
{
    return m_name;
}

QString SensorInputItem::fullName() const
{
    QString fn;
    if(m_pParent && m_pParent->parent()){
        fn = m_pParent->fullName();
        fn += ".";
    }
    fn += m_name;
    return fn;
}

const QString SensorInputItem::address() const
{
    QString fa;
    if(m_pParent && m_pParent->parent()){
        fa = m_pParent->address();
        fa += ".";
    }
    fa += m_address;
    return fa;
}

void SensorInputItem::setName(QString name)
{
    m_name = name;
}

int SensorInputItem::childCount() const
{
    return m_childs.count();
}

int SensorInputItem::childNumber() const
{
    if(m_pParent){
        return m_pParent->childs().indexOf(const_cast<SensorInputItem *>(this));
    }
    return -1;
}

QString SensorInputItem::sortId() const
{
    QString strId;
    if(m_pParent){
        strId = m_pParent->sortId();
    }
    int id = childNumber();
    if(id < 0){
        id = 0;
    }
    return strId + QString::number(id);
}

void SensorInputItem::enable()
{
    m_enabled = true;
}

void SensorInputItem::disable()
{
    m_enabled = false;
}

bool SensorInputItem::enabled() const
{
    return m_enabled;
}




