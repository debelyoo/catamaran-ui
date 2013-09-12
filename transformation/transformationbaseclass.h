#ifndef TRANSFORMATIONBASECLASS_H
#define TRANSFORMATIONBASECLASS_H

#include <QtCore>
#include "communication/dataObject.h"
#include "communication/idatamessagereceiver.h"

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

    TransformationBaseClass();
    virtual const QVector<ParameterDescription> getParametersDefinition() const = 0;
    virtual const TransformationDefinition getTransformationDefinition() const = 0;
    virtual const DataObject applyTransform(DataObject val, IDataMessageReceiver* callback) = 0;

    void setParameters(const QVector<QVariant> params);
    QVector<QVariant> getParameters() const;
protected:
    QVector<QVariant> parameters;
};

#endif // TRANSFORMATIONBASECLASS_H
