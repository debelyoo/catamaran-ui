#include "mission.h"

Mission::Mission(qint64 id, QString n, QDateTime depTime, QString tz, QString v)
{
    this->id = id;
    this->name = n;
    this->departureTime = depTime;
    this->timeZone = tz;
    this->vehicle = v;
}

Mission::Mission(){}

qint64 Mission::getId()
{
    return id;
}
