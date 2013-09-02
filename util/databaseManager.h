#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QMap>

namespace Datastore {
    typedef enum {
        TemperatureLog = 0,
        WindLog = 1,
        RadiometerLog = 2
    } Tables;
}

class DatabaseManager
{

    public:
        static DatabaseManager* instance();
        //bool openDB();
        bool createLogTableForDoubleValue(QString tableName);
        bool insertLogDoubleValue(QString tableName, int address, qint64 ts, double value);
        bool getTemperatureLog();
        QString getTableName(Datastore::Tables);

    private:
        DatabaseManager(){
            tables.insert(0, "temperaturelog");
            tables.insert(1, "windlog");
            tables.insert(2, "radiometerlog");

            // Find SQLite driver
            db = QSqlDatabase::addDatabase("QSQLITE");
            db.setDatabaseName("ecoldata.sqlite");
            createLogTableForDoubleValue(getTableName(Datastore::TemperatureLog));
            createLogTableForDoubleValue(getTableName(Datastore::WindLog));
            createLogTableForDoubleValue(getTableName(Datastore::RadiometerLog));
        }
        DatabaseManager(const DatabaseManager &);
        DatabaseManager& operator=(const DatabaseManager &);

        static DatabaseManager* m_Instance;

        QSqlDatabase db;
        QMap<int, QString> tables;

};

#endif // DATABASEMANAGER_H
