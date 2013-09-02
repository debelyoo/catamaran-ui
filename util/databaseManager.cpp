#include "databaseManager.h"
#include <QSqlQuery>
#include <QDebug>

DatabaseManager* DatabaseManager::m_Instance = 0;

/** This function is called to create an instance of the class.
    Calling the constructor publicly is not allowed. The constructor
    is private and is only called by this Instance function.
*/
DatabaseManager* DatabaseManager::instance()
{
    if (!m_Instance)   // Only allow one instance of class to be generated.
    {
        m_Instance = new DatabaseManager;
    }
    return m_Instance;
}

QString DatabaseManager::getTableName(Datastore::Tables t)
{
    return tables[t];
}

/*bool DatabaseManager::openDB()
{
    // open database
    return db.open();
}*/

bool DatabaseManager::createLogTableForDoubleValue(QString tableName)
{
    bool res;
    if (db.open()) {
        QSqlQuery query(db);
        if (query.exec("CREATE TABLE IF NOT EXISTS "+ tableName +" ( id INTEGER PRIMARY KEY AUTOINCREMENT, sensor_address INTEGER, timestamp INTEGER, value REAL );")) {
            res = true;
            qDebug() << "table creation succeeded !";
        } else {
            res = false;
            qDebug() << "table creation failed !" << query.lastError();
        }
    } else {
        res = false;
        qDebug() << "DB not open !";
    }
    db.close();
    return res;
}

bool DatabaseManager::insertLogDoubleValue(QString tableName, int address, qint64 ts, double value)
{
    bool res;
    if (db.open()) {
        QSqlQuery query(db);
        query.prepare("INSERT INTO "+ tableName +" ( sensor_address, timestamp, value ) VALUES (?, ?, ?);");
        query.bindValue(0, address);
        query.bindValue(1, ts);
        query.bindValue(2, value);
        if (query.exec()) {
            res = true;
            qDebug() << "insertion succeeded !";
        } else {
            res = false;
            qDebug() << "insertion failed !" << query.lastError();
        }
    } else {
        res = false;
        qDebug() << "DB not open !";
    }
    db.close();
    return res;
}

bool DatabaseManager::getTemperatureLog()
{
    bool res;
    if (db.open()) {
        QSqlQuery query(db);
        if (query.exec("SELECT * FROM temperaturelog;")) {
            while( query.next() )
            {
                double logValue = query.value( 3 ).toDouble();
                QString msg = "select succeeded ! - ["+ QString::number(logValue) +"]";
                qDebug() << msg;
            }
        } else {
            res = false;
            qDebug() << "select failed !" << query.lastError();
        }
    } else {
        res = false;
        qDebug() << "DB not open !";
    }
    db.close();
    return res;
}
