#ifndef SENSOR_H
#define SENSOR_H

#include <QObject>
#include "sensorType.h"
#include <QList>
#include "transformation/transformationbaseclass.h"
#include "sensorConfig.h"
#include "manager/sensortypemanager.h"
#include "transformation/transformationmanager.h"

class RegisteredSensorItem;
class TransformationBaseClass;
class Sensor
{
public:
    Sensor(QString addr = "", QString name = "",
                    const SensorType* type = 0,
                    bool record = false, bool stream = false,
                    QString logFilePrefix = "", QString currentLogFilename = "", bool isData = true);
//    explicit Sensor(Sensor *parent, QString addr = "", QString name = "",
//                    SensorType* type = 0, int display = 0,
//                    bool record = false, bool stream = false,
//                    QString logFilePrefix = "", QString currentLogFilename = "");

    Sensor(QDataStream &ds);
    ~Sensor();

    // Getters
    bool isData() const;
    int display() const;
    const QString &address() const;
    const QString &name() const;
    const SensorType *type() const;
    bool stream() const;
    bool record() const;
    const TransformationBaseClass *transformation() const;
    const QString &logFilePrefix() const;
    const QString &currentLogFilename() const;

    QString fullname() const;

    // Setters
    void setName(const QString &name);
    void setDisplay(int g);
    void setType(const SensorType *type);
    void setStream(bool enable);
    void setRecord(bool enable);
    void setTransformation(TransformationBaseClass *tr);
    void setLogFilePrefix(const QString &prefix);
    void setCurrentLogFilename(const QString &fn);

    friend QDataStream &operator<< (QDataStream &stream, const Sensor &s);
    friend class RegisteredSensorItem;
    // Modifiers

protected:
    bool addChild(Sensor *child, bool checkDuplicate = true);

private:

    QString             m_address;
    QString             m_name;

    bool                m_isData;
    const SensorType    *m_type;
    bool                m_record;
    bool                m_stream;
    TransformationBaseClass *m_transformation;


    QString             m_logFilePrefix;
    QString             m_currentLogFilename;

};

QDataStream &operator<< (QDataStream &stream, const Sensor &s);

Q_DECLARE_METATYPE(Sensor)
Q_DECLARE_METATYPE(QList<Sensor *>)

#endif // SENSOR_H
