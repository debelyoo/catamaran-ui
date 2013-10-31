#include "registeredsensoritem.h"

#include "manager/sensortypemanager.h"

RegisteredSensorItem::RegisteredSensorItem(SensorInputItem *sensorInput):
    m_parent(NULL),
    m_sensorInput(sensorInput),
    m_childs(),
    m_name(sensorInput->fullName()),
    m_transformation(NULL),
    m_sortId(sensorInput->sortId())
{
    m_model = new Sensor(sensorInput->address(), sensorInput->name());
    m_sensorInput->disable();
}

RegisteredSensorItem::RegisteredSensorItem(SensorInputItem *sensorInput, Sensor *s):
    m_parent(NULL),
    m_sensorInput(sensorInput),
    m_childs(),
    m_name(sensorInput->fullName()),
    m_transformation(NULL),
    m_sortId(sensorInput->sortId()),
    m_model(s)
{
    setTransformation(s->m_transformation);
    setRecord(s->record());
    setStream(s->stream());
    setName(s->name());
    m_sensorInput->disable();
}

RegisteredSensorItem::~RegisteredSensorItem()
{
    SensorConfig::instance()->removeSensor(m_model->address());
    if(m_parent){
        m_parent->removeChild(this);
    }
    m_sensorInput->enable();
    delete m_model;
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

void RegisteredSensorItem::addChild(RegisteredSensorItem *child)
{
    m_childs.append(child);
    child->setParent(this);
}

void RegisteredSensorItem::insertChild(RegisteredSensorItem *child, int pos)
{
    m_childs.insert(pos, child);
    child->setParent(this);
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
    return m_model->name();
}

void RegisteredSensorItem::setName(QString name)
{
    //m_name = name;
    m_model->setName(name);
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
    case 3:
        return QVariant(m_model->stream());
    case 4:
        return QVariant(m_model->record());
    default:
        return QVariant();
    }
}

QString RegisteredSensorItem::sortId() const
{
    return m_sortId;
}

TransformationBaseClass *RegisteredSensorItem::transformation() const
{
    return m_transformation;
}

bool RegisteredSensorItem::setTransformation(TransformationBaseClass *transformation)
{
    bool deleted = false;
    if(!m_transformation){
        if(transformation){
            m_transformation = transformation;
            m_model->setTransformation(m_transformation);
            foreach(TransformationBaseClass::SubSensor s, transformation->getSubSensors()){
                SensorInputItem *subsensor = new SensorInputItem(s.name);
                m_sensorInput->addChild(subsensor);
            }
        }
    }else{
        if(transformation){
            if(m_transformation->getTransformationDefinition().name != transformation->getTransformationDefinition().name){
                delete m_transformation;
                deleted = true;
                m_transformation = transformation;
                m_model->setTransformation(m_transformation);
                foreach(TransformationBaseClass::SubSensor s, transformation->getSubSensors()){
                    SensorInputItem *subsensor = new SensorInputItem(s.name);
                    m_sensorInput->addChild(subsensor);
                }
            }
        }else{
            delete m_transformation;
            m_transformation = NULL;
            m_model->setTransformation(m_transformation);
            deleted = true;
        }
    }
    return deleted;
}

void RegisteredSensorItem::setStream(bool stream)
{
    m_model->setStream(stream);
}

void RegisteredSensorItem::setRecord(bool rec)
{
    m_model->setRecord(rec);
}

void RegisteredSensorItem::setType(const QString &type){
    m_model->setType(SensorTypeManager::instance()->type(type));
}

void RegisteredSensorItem::setLogFilePrefix(const QString &pref)
{
    m_model->setLogFilePrefix(pref);
}

const Sensor *RegisteredSensorItem::sensor() const
{
    return m_model;
}

const SensorInputItem *RegisteredSensorItem::sensorInpurItem() const
{
    return m_sensorInput;
}




