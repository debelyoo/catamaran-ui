#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDateTime>
#include "model/sensor.h"

/*namespace Datastore {
    typedef enum {
        TemperatureLog = 0,
        WindLog = 1,
        RadiometerLog = 2
    } Tables;
}*/

class DatabaseManager
{

    public:
        static DatabaseManager* instance();
        //bool openDB();
        bool createLogTableForDoubleValue(QString tableName);
        bool insertLogDoubleValue(QString tableName, int address, qint64 ts, double value);
        bool getTemperatureLog();
        QPair< QVector<double>, QVector<double> > getData(Sensor* s, int fromTs);
        //QString getTableName(Datastore::Tables);

    private:
        DatabaseManager(){
            // Find SQLite driver
            db = QSqlDatabase::addDatabase("QSQLITE");
            db.setDatabaseName("ecoldata.sqlite");
        }
        DatabaseManager(const DatabaseManager &);
        DatabaseManager& operator=(const DatabaseManager &);

        static DatabaseManager* m_Instance;

        QSqlDatabase db;
        //QList<QString> tables;

};

#endif // DATABASEMANAGER_H
