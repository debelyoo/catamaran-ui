#include "prisme.h"

PRisme::PRisme() :
    TransformationBaseClass(),
    m_subsensorsAddress()
{
	/*
		Do not touch the constructor if you have no decent reasons.
	*/
	QVector<ParameterDescription> defaultParams = getParametersDefinition();
    for(int i=0; i<defaultParams.size(); ++i){
        parameters.append(defaultParams[i].typeAndDefault);
    }
    foreach(SubSensor ss, getSubSensors()){
        m_subsensorsAddress.append(ss.name);
    }
}

TransformationBaseClass *PRisme::newInstance() const{
    /*
        Do not touch the constructor if you have no decent reasons.
    */
    return new PRisme();
}

const CRioData PRisme::applyTransform(CRioData obj, IDataMessageReceiver* callback, const AbstractCrioStatesHolder *crioStates, const AbstractSensorsDataHolder *sensorsData) const
{
    Q_UNUSED(crioStates);
    Q_UNUSED(sensorsData);
    return PRismeMessage::decodePrismeData(obj, callback, obj.address, m_subsensorsAddress);
}

const QVector<TransformationBaseClass::ParameterDescription> PRisme::getParametersDefinition() const{
	QVector<ParameterDescription> params;

    return params;
}

const TransformationBaseClass::TransformationDefinition PRisme::getTransformationDefinition() const{
    TransformationDefinition a;
    a.name = "PRisme Data Decoder";
    a.description = "Parse and decode byte array coming from the PRisme and reinject them as new data";
    return a;
}

const QVector<TransformationBaseClass::SubSensor> PRisme::getSubSensors() const
{

    QVector<TransformationBaseClass::SubSensor> subs;

    SubSensor s;
    s.name = "ADC0";
    s.description = "Analog to digital converter 0";
    subs.append(s);
    s.name = "ADC1";
    s.description = "Analog to digital converter 1";
    subs.append(s);
    s.name = "ADC2";
    s.description = "Analog to digital converter 2";
    subs.append(s);
    s.name = "ADC3";
    s.description = "Analog to digital converter 3";
    subs.append(s);
    s.name = "ADC4";
    s.description = "Analog to digital converter 4";
    subs.append(s);
    s.name = "ADC5";
    s.description = "Analog to digital converter 5";
    subs.append(s);
    s.name = "ADC6";
    s.description = "Analog to digital converter 6";
    subs.append(s);
    s.name = "ADC7";
    s.description = "Analog to digital converter 7";
    subs.append(s);

    return subs;
}

