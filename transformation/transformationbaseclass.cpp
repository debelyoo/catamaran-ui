#include "transformationbaseclass.h"

TransformationBaseClass::TransformationBaseClass(): parameters(){
}

void TransformationBaseClass::setParameters(const QVector<QVariant> params){
    parameters = params;
}

QVector<QVariant> TransformationBaseClass::getParameters() const{
    return parameters;
}
