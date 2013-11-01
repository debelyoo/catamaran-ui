#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QPair>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDateTime>
#include <QMessageBox>
#include <QStandardItemModel>
#include "model/sensor.h"
#include "model/mission.h"
#include "dbTable.h"
#include "manager/sensortypemanager.h"
#include "util/timeHelper.h"

namespace TableList {
    typedef enum {
        MISSION = 0,
        DATA_FOR_MISSION = 1,
        GPS_LOG = 2,
        SENSOR_LOG = 3,
        SENSOR_CONFIG = 4
    } Tables;
}

class DatabaseManager
{

    public:
        static DatabaseManager* instance();
        ///
        QString getCurrentMissionName();
        bool createTable(TableList::Tables tableId, DbTable table);
        bool insertGpsPoint(qint64 unixTs, double lat, double lon, double alt, double heading);
        bool insertSensorValue(QString sensorAddress, QString sensorType, qint64 unixTs, double value);
        bool insertSensorConfigBlob(QByteArray blob);
        bool insertMission();
        bool removeMission(QString missionName);
        QStandardItemModel* getMissionsAsModel();
        QStandardItemModel* getDataForMissionsAsModel(QString missionName);
        void insertSampleData(); // TODO - TEST only
        ///
        bool getTemperatureLog();
        QPair< QVector<double>, QVector<double> > getData(Sensor* s, int fromTs);
        QList<QJsonObject> getDataAsJSON(QString missionName, QString sensorType, long missionIdOnBackend);
        QPair<int, QJsonDocument> getMissionAsJSON(QString missionName);
        QByteArray getSensorConfigBlob(qint64 missionId);

    private:
        DatabaseManager(){
            // Find SQLite driver
            db = QSqlDatabase::addDatabase("QSQLITE");
            db.setDatabaseName("ecoldata.sqlite");
            createNecessaryTables();
            insertMission(); // create mission when application starts
            //insertSampleData();
        }
        DatabaseManager(const DatabaseManager &);
        DatabaseManager& operator=(const DatabaseManager &);

        static DatabaseManager* m_Instance;

        QSqlDatabase db;
        QMap<TableList::Tables, DbTable> tables;
        qint64 currentMissionId; // stores the id of the current mission
        QString currentMissionName; // stores the name of the current mission

        QString buildCreateQuery(DbTable table);
        QString buildInsertQuery(DbTable table);
        void createNecessaryTables();
        Mission getMission(QString missionName);
        QPair<int, QList<Sensor*> > getSensorsForMission(QString missionName);
        bool insertRecord(DbTable table, QList<QVariant> values);
        bool removeSensorConfigBlobForMission(qint64 missionId);

};

#endif // DATABASEMANAGER_H
