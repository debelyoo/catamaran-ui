#ifndef MISSION_H
#define MISSION_H

#include <QObject>
#include <QDateTime>

class Mission {
    public:
        Mission(qint64 id, QString n, QDateTime departureTime, QString timeZone, QString vehicle);
        Mission();
        qint64 getId();

    private:
        qint64 id;
        QString name;
        QDateTime departureTime;
        QString timeZone;
        QString vehicle;
};

#endif // MISSION_H
