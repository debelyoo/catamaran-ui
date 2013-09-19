#ifndef REGISTEREDSENSORITEM_H
#define REGISTEREDSENSORITEM_H

#include <QVector>
#include <QString>
#include "transformation/transformationbaseclass.h"
#include "sensorinputitem.h"

class RegisteredSensorItem
{
public:
    RegisteredSensorItem(SensorInputItem *sensorInput, RegisteredSensorItem *parent = NULL);
    RegisteredSensorItem *parent() const;
    void setParent(RegisteredSensorItem *parent);

    QVector<RegisteredSensorItem *> childs() const;
    void setChilds(const QVector<RegisteredSensorItem *> &childs);
    void addChild(RegisteredSensorItem *child);
    void insertChild(RegisteredSensorItem *child, int pos);
    RegisteredSensorItem *removeChild(RegisteredSensorItem *child);
    RegisteredSensorItem *child(int index) const;

    QString name() const;
    void setName(QString name);

    int childCount() const;
    int childNumber() const;
    QVariant data(int col) const;
    QString sortId();

protected:
    RegisteredSensorItem *m_parent;
    QVector<RegisteredSensorItem *> m_childs;
    QString m_name;
    TransformationBaseClass *m_transformation;

    QString m_sortId;
};

#endif // REGISTEREDSENSORITEM_H
