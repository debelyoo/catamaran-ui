#include "sensor.h"

Sensor::Sensor(QString addr, QString name, const SensorType* type, bool record, bool stream, QString logFilePrefix, QString currentLogFilename, bool isData):
    m_address(addr),
    m_name(name),
    m_isData(isData),
    m_type(type),
    m_record(record),
    m_stream(stream),
    m_transformation(NULL),
    m_logFilePrefix(logFilePrefix),
    m_currentLogFilename(currentLogFilename)
{
    SensorConfig::instance()->addSensor(this);
}

Sensor::Sensor(QDataStream &ds)
{
    ds >> m_address;
    ds >> m_name;
    ds >> m_isData;
    QString typeName;
    ds >> typeName;
    const SensorType *type = SensorTypeManager::instance()->type(typeName);
    if(type){
        m_type = type;
    }else{
        m_type = NULL;
    }
    ds >> m_record;
    ds >> m_stream;
    QString transformationName;
    ds >> transformationName;
    QVector<QVariant> transformationParameters;
    ds >> transformationParameters;

    m_transformation = TransformationManager::instance()->getTransformation(transformationName);
    if(m_transformation){
        m_transformation = m_transformation->newInstance();
        m_transformation->setParameters(transformationParameters);
    }
    ds >> m_logFilePrefix;
    ds >> m_currentLogFilename;
    SensorConfig::instance()->addSensor(this);
}

Sensor::~Sensor()
{
    SensorConfig::instance()->removeSensor(m_address);
}

bool Sensor::isData() const
{
    return m_isData;
}


const QString &Sensor::address() const
{
    return m_address;
}

const QString &Sensor::name() const
{
    return m_name;
}

const SensorType *Sensor::type() const
{
    return m_type;
}

bool Sensor::stream() const
{
    return m_stream;
}

bool Sensor::record() const
{
    return m_record;
}

const TransformationBaseClass *Sensor::transformation() const
{
    return m_transformation;
}

const QString &Sensor::logFilePrefix() const
{
    return m_logFilePrefix;
}

const QString &Sensor::currentLogFilename() const
{
    return m_currentLogFilename;
}

void Sensor::setName(const QString &name)
{
    m_name = name;
}


void Sensor::setType(const SensorType *type)
{
    m_type = type;
}

void Sensor::setStream(bool enable)
{
    m_stream = enable;
}

void Sensor::setRecord(bool enable)
{
    m_record = enable;
}

void Sensor::setTransformation(TransformationBaseClass *tr)
{
    m_transformation = tr;
}

void Sensor::setLogFilePrefix(const QString &prefix)
{
    m_logFilePrefix = prefix;
}

void Sensor::setCurrentLogFilename(const QString &fn)
{
    m_currentLogFilename = fn;
}


QDataStream &operator<<(QDataStream &ds, const Sensor &s)
{
    ds << s.m_address;
    ds << s.m_name;
    ds << s.m_isData;
    if(s.m_type){
        ds << s.m_type->name();
    }else{
        ds << QString("Unknown");
    }
    ds << s.m_record;
    ds << s.m_stream;
    if(s.m_transformation){
        ds << s.m_transformation->getTransformationDefinition().name;
        ds << s.m_transformation->getParameters();
    }else{
        ds << QString("___");
        ds << QVector<QVariant>();
    }
    ds << s.m_logFilePrefix;
    ds << s.m_currentLogFilename;
    return ds;
}
