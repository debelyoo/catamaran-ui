#include "transformationbaseclass.h"

TransformationBaseClass::TransformationBaseClass(): parameters(){
}

TransformationBaseClass::~TransformationBaseClass()
{
}

void TransformationBaseClass::setParameters(const QVector<QVariant> params){
    parameters = params;
}

QVector<QVariant> TransformationBaseClass::getParameters() const{
    return parameters;
}
