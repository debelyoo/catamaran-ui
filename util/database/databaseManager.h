#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDateTime>
#include <QMessageBox>
#include <QStandardItemModel>
#include "model/sensor.h"
#include "model/mission.h"
#include "dbTable.h"
#include "manager/sensortypemanager.h"

class DatabaseManager
{

    public:
        static DatabaseManager* instance();
        //bool createLogTableForDoubleValue(QString tableName);
        //bool insertLogDoubleValue(QString tableName, int address, qint64 ts, double value);
        ///
        bool createTableByTemplate(QString templateName);
        bool createTable(DbTable table);
        bool insertValue(DbTable table, QList<QVariant> values);
        bool insertMission();
        bool setDatatypesForCurrentMission();
        bool addDatatypeForCurrentMission(QString datatype);
        QStandardItemModel* getMissionsAsModel();
        QStandardItemModel* getDataForMissionsAsModel(QString missionName);
        void insertSampleData(); // TODO - TEST only
        ///
        bool getTemperatureLog();
        QPair< QVector<double>, QVector<double> > getData(Sensor* s, int fromTs);
        QJsonDocument getDataAsJSON(QString missionName, const SensorType* st, long missionIdOnBackend);
        QJsonDocument getMissionAsJSON(QString missionName);

    private:
        DatabaseManager(){
            // Find SQLite driver
            db = QSqlDatabase::addDatabase("QSQLITE");
            db.setDatabaseName("ecoldata.sqlite");
            //db.open();
            createNecessaryTables();
            insertMission(); // create mission when application starts
            addDatatypeForCurrentMission("GPS");
            addDatatypeForCurrentMission("temperature");
        }
        DatabaseManager(const DatabaseManager &);
        DatabaseManager& operator=(const DatabaseManager &);

        static DatabaseManager* m_Instance;

        QSqlDatabase db;
        QMap<QString, DbTable> tables;
        qint64 currentMissionId; // stores the id of the current mission

        QString buildCreateQuery(DbTable table);
        QString buildInsertQuery(DbTable table);
        void createNecessaryTables();
        Mission getMission(QString missionName);

};

#endif // DATABASEMANAGER_H
