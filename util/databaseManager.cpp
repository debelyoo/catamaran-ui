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

/*QString DatabaseManager::getTableName(Datastore::Tables t)
{
    return tables[t];
}*/

/*bool DatabaseManager::openDB()
{
    // open database
    return db.open();
}*/

/**
 * Create a table in embedded Sqlite DB (id, sensor_address, timestamp (double), value (double))
 * @brief DatabaseManager::createLogTableForDoubleValue
 * @param tableName The name of the table to create
 * @return true if table has been created successfully
 */
bool DatabaseManager::createLogTableForDoubleValue(QString tableName)
{
    bool res;
    if (db.open()) {
        QSqlQuery query(db);
        if (query.exec("CREATE TABLE IF NOT EXISTS "+ tableName +" ( id INTEGER PRIMARY KEY AUTOINCREMENT, sensor_address INTEGER, timestamp REAL, value REAL );")) {
            res = true;
            //qDebug() << "table creation succeeded ! "+ tableName;
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

/**
 * Insert log value (double) in database
 * @brief DatabaseManager::insertLogDoubleValue
 * @param tableName The name of the table in DB
 * @param address The address of the sensor
 * @param ts The timestamp of the log
 * @param value The value of the log
 * @return true if success
 */
bool DatabaseManager::insertLogDoubleValue(QString tableName, int address, qint64 ts, double value)
{
    bool res;
    if (db.open()) {
        // convert ts (milli) to ts (seconds) so that it can be stored as double
        double tsSec = (double)ts / 1000;
        QSqlQuery query(db);
        query.prepare("INSERT INTO "+ tableName +" ( sensor_address, timestamp, value ) VALUES (?, ?, ?);");
        query.bindValue(0, address);
        query.bindValue(1, tsSec);
        query.bindValue(2, value);
        if (query.exec()) {
            res = true;
            //qDebug() << "insertion succeeded !";
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

/**
 * Get data from database for a specific sensor from a specific timestamp
 * @brief DatabaseManager::getData
 * @param s The sensor for which getting the data
 * @param fromTs The timestamp from which getting the data (in seconds)
 * @return A pair of vectors: one containing times (ts in seconds), and one containing values
 */
QPair< QVector<double>, QVector<double> > DatabaseManager::getData(Sensor* s, int fromTs)
{
    QVector<double> logTimes;
    QVector<double> logValues;
    if (db.open()) {
        QSqlQuery query(db);
        QString sqlQuery = "SELECT * FROM "+ s->getType()->getDbTableName() +" WHERE sensor_address = "+ QString::number(s->getAddress());
        sqlQuery += " AND timestamp > "+ QString::number(fromTs);
        sqlQuery += " ORDER BY timestamp";
        sqlQuery += ";";
        //qDebug() << "SQL query: " << sqlQuery;
        if (query.exec(sqlQuery)) {
            while( query.next() )
            {
                logTimes.push_back(query.value( 2 ).toDouble()); // times are in seconds to fit with QCustomPlot
                logValues.push_back(query.value( 3 ).toDouble());
            }
        } else {
            qDebug() << "select failed !" << query.lastError();
        }
    } else {
        qDebug() << "DB not open !";
    }
    db.close();
    /*if (logValues.size() > 0)
    {
        qDebug() << "getData() - values: "+ QString::number(logValues.size())+" for sensor address: "+ QString::number(s->getAddress());
        //qDebug() << "getData() - last time: "+ QString::number(logTimes.last(), 'f', 10)+" last value: "+ QString::number(logValues.last());
    }*/
    QPair< QVector<double>, QVector<double> > data = QPair< QVector<double>, QVector<double> >(logTimes, logValues);
    return data;
}
