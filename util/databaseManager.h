#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>

class DatabaseManager
{

    public:
        static DatabaseManager* instance();
        //bool openDB();
        bool createTemperatureLogTable();
        bool insertTemperatureLog(int address, int ts, double value);
        bool getTemperatureLog();

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

};

#endif // DATABASEMANAGER_H
