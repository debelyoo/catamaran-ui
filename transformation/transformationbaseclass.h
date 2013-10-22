#ifndef TRANSFORMATIONBASECLASS_H
#define TRANSFORMATIONBASECLASS_H

#include <QtCore>
//#include "communication/dataObject.h"
#include "transformation/abstractcriostatesholder.h"
#include "transformation/abstractsensorsdataholder.h"
#include "communication/idatamessagereceiver.h"
#include "communication/criodata.h"

class TransformationBaseClass
{
public:
    typedef struct{
        QVariant typeAndDefault;
        QString description;
        QString name;
        bool (*validationFunction)(QVariant, QString&);
    } ParameterDescription;
    typedef struct{
        QString name;
        QString description;
    } TransformationDefinition;
    typedef struct{
        QString name;
        QString description;
    } SubSensor;

    TransformationBaseClass();
    virtual ~TransformationBaseClass();
    virtual const QVector<ParameterDescription> getParametersDefinition() const = 0;
    virtual const TransformationDefinition getTransformationDefinition() const = 0;
    virtual const CRioData applyTransform(CRioData val, IDataMessageReceiver* callback, const AbstractCrioStatesHolder *crioStates, const AbstractSensorsDataHolder *sensorsData) const = 0;
    virtual const QVector<SubSensor> getSubSensors() const = 0;
    virtual TransformationBaseClass *newInstance() const = 0;

    void setParameters(const QVector<QVariant> params);
    QVector<QVariant> getParameters() const;
protected:
    QVector<QVariant> parameters;
};

#endif // TRANSFORMATIONBASECLASS_H
