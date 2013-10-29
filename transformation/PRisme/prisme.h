#ifndef PRISME_H
#define PRISME_H

//#include "prisme_global.h"
#include <QVariant>
#include <QStringList>
#include "transformation/transformationbaseclass.h"
#include "prismemessage.h"


class PRisme : public TransformationBaseClass{
public:
    PRisme();
    const QVector<ParameterDescription> getParametersDefinition() const;
    const TransformationDefinition getTransformationDefinition() const;
    const CRioData applyTransform(CRioData obj, IDataMessageReceiver* callback, const AbstractCrioStatesHolder *crioStates = 0, const AbstractSensorsDataHolder *sensorsData = 0) const;
    const QVector<SubSensor> getSubSensors() const;
    TransformationBaseClass *newInstance() const;
private:
    QStringList m_subsensorsAddress;
};

#endif // PRISME_H
