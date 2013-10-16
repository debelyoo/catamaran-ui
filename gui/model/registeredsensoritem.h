#ifndef REGISTEREDSENSORITEM_H
#define REGISTEREDSENSORITEM_H

#include <QVector>
#include <QString>
#include "transformation/transformationbaseclass.h"
#include "sensorinputitem.h"
#include "model/sensor.h"
#include "sensorConfig.h"

class RegisteredSensorItem
{
public:
    RegisteredSensorItem(SensorInputItem *sensorInput, RegisteredSensorItem *parent = NULL);
    ~RegisteredSensorItem();

    RegisteredSensorItem *parent() const;
    void setParent(RegisteredSensorItem *parent);

    QVector<RegisteredSensorItem *> childs() const;
    void addChild(RegisteredSensorItem *child);
    void insertChild(RegisteredSensorItem *child, int pos);
    RegisteredSensorItem *removeChild(RegisteredSensorItem *child);
    RegisteredSensorItem *child(int index) const;

    QString name() const;
    void setName(QString name);

    int childCount() const;
    int childNumber() const;
    QVariant data(int col) const;
    QString sortId() const;
    TransformationBaseClass *transformation() const;
    bool setTransformation(TransformationBaseClass *transformation);

    void setStream(bool stream);
    void setRecord(bool rec);
    void setType(const QString &type);

    const Sensor *sensor() const;
protected:
    RegisteredSensorItem *m_parent;
    SensorInputItem *m_sensorInput;
    QVector<RegisteredSensorItem *> m_childs;
    QString m_name;
    TransformationBaseClass *m_transformation;

    QString m_sortId;

    Sensor *m_model;
};

#endif // REGISTEREDSENSORITEM_H
