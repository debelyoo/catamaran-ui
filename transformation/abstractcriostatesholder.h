#ifndef ABSTRACTCRIOSTATESHOLDER_H
#define ABSTRACTCRIOSTATESHOLDER_H

#include <QPointF>

class AbstractCrioStatesHolder
{
public:
    AbstractCrioStatesHolder(){}
    virtual qint8 leftEngineValue() const = 0;
    virtual qint8 rightEngineValue() const = 0;
    virtual double heading() const = 0;
    virtual QPointF position() const = 0;
    virtual QPointF speed() const = 0;
    virtual QPointF meanSpeed() const = 0;
};

#endif // ABSTRACTCRIOSTATESHOLDER_H
