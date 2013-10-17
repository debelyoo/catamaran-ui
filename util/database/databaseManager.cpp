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

/**
 * Create the necessary tables that are always needed
 * @brief DatabaseManager::createNecessaryTables
 */
void DatabaseManager::createNecessaryTables()
{
    // create mission table
    QList<DbColumn> cols;
    cols.append(DbColumn("mission_id", SQLite::TEXT));
    cols.append(DbColumn("departure_time", SQLite::REAL));
    DbTable missionTable = DbTable("mission", cols);
    createTable(missionTable);
    // create dataformission table
    cols.clear();
    cols.append(DbColumn("mission_id", SQLite::INTEGER));
    cols.append(DbColumn("datatype", SQLite::TEXT));
    DbTable dfmTable = DbTable("dataformission", cols);
    createTable(dfmTable);

}

/**
 * Insert values in SQLite database
 * @brief DatabaseManager::insertValue
 * @param table The table in which inserting the values
 * @param values The values to insert (MUST be in the same order as table columns)
 * @return True if success
 */
bool DatabaseManager::insertValue(DbTable table, QList<QVariant> values)
{
    bool res;
    if (table.getColumns().length() != values.length()) {
        qDebug() << "[DatabaseManager.insertValue()] [ERROR] nb of columns and values do not match !";
        return false;
    }
    QString queryStr = buildInsertQuery(table);
    if (db.open()) {
        // convert ts (milli) to ts (seconds) so that it can be stored as double
        // double tsSec = (double)ts / 1000;
        QSqlQuery query(db);
        query.prepare(queryStr);
        for (int i = 0; i<table.getColumns().length(); i++) {
            query.bindValue(i, values.at(i));
        }
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

bool DatabaseManager::createTableByTemplate(QString templateName)
{
    DbTable table;
    QList<DbColumn> cols;
    if (templateName == "gpslog") {
        cols.append(DbColumn("sensor_address", SQLite::INTEGER));
        cols.append(DbColumn("timestamp", SQLite::REAL));
        cols.append(DbColumn("latitude", SQLite::REAL));
        cols.append(DbColumn("longitude", SQLite::REAL));
        cols.append(DbColumn("altitude", SQLite::REAL));
        cols.append(DbColumn("heading", SQLite::REAL));
        table = DbTable(templateName, cols);
    } else if (templateName == "temperaturelog") {
        cols.append(DbColumn("sensor_address", SQLite::INTEGER));
        cols.append(DbColumn("timestamp", SQLite::REAL));
        cols.append(DbColumn("value", SQLite::REAL));
        table = DbTable(templateName, cols);
    } else {
        qDebug() << "[DatabaseManager.createTableByTemplate()] Unknown template table";
        return false;
    }
    return createTable(table);
}

/**
 * Create a table in SQLite database
 * @brief DatabaseManager::createTable
 * @param table The table to create
 * @return True if success
 */
bool DatabaseManager::createTable(DbTable table)
{
    bool res;
    QString queryStr = buildCreateQuery(table);
    if (db.open()) {
        QSqlQuery query(db);
        if (query.exec(queryStr)) {
            tables.insert(table.getName(), table);
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

QString DatabaseManager::buildCreateQuery(DbTable table)
{
    QString query = "CREATE TABLE IF NOT EXISTS "+ table.getName();
    query += " ( id INTEGER PRIMARY KEY AUTOINCREMENT";
    foreach (DbColumn col, table.getColumns()) {

        query += ", "+ col.getName() + " " + col.getTypeAsString();
    }
    query += " );";
    //qDebug() << query;
    return query;
}

QString DatabaseManager::buildInsertQuery(DbTable table)
{
    // "INSERT INTO mytable ( sensor_address, timestamp, value ) VALUES (?, ?, ?);"
    QString query = "INSERT INTO "+ table.getName();
    QString fields, values = "";
    int inc = 0;
    foreach (DbColumn col, table.getColumns()) {
        if (inc == 0)
        {
            fields += col.getName();
            values += "?";
        }
        else
        {
            fields += ", "+col.getName();
            values += ", ?";
        }
        inc++;
    }
    query += " ( "+ fields +" ) VALUES ("+ values +");";
    return query;
}

/**
 * Create a table in embedded Sqlite DB (id, sensor_address, timestamp (double), value (double))
 * @brief DatabaseManager::createLogTableForDoubleValue
 * @param tableName The name of the table to create
 * @return true if table has been created successfully
 */
/*bool DatabaseManager::createLogTableForDoubleValue(QString tableName)
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
}*/

/**
 * Insert log value (double) in database
 * @brief DatabaseManager::insertLogDoubleValue
 * @param tableName The name of the table in DB
 * @param address The address of the sensor
 * @param ts The timestamp of the log
 * @param value The value of the log
 * @return true if success
 */
/*bool DatabaseManager::insertLogDoubleValue(QString tableName, int address, qint64 ts, double value)
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
}*/

/*bool DatabaseManager::getTemperatureLog()
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
}*/

bool DatabaseManager::insertMission()
{
    // mission_id - departure time
    QList<QVariant> values;
    QDateTime dateTime = QDateTime::currentDateTime();
    QString missionName = "mission_"+dateTime.toString("yyyyMMdd_hh:mm:ss");;
    currentMission = missionName;
    double ts = (double)dateTime.currentMSecsSinceEpoch() / 1000;
    values.append(missionName);
    values.append(ts);
    return insertValue(tables["mission"], values);
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
        QString sqlQuery = "SELECT * FROM "+ s->type()->getDbTableName() +" WHERE sensor_address = "+ s->address();
        sqlQuery += " AND timestamp > "+ QString::number(fromTs);
        sqlQuery += " ORDER BY timestamp";// LIMIT 100";
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

/**
 * Get the missions in DB, return them as a model for a list view
 * @brief DatabaseManager::getMissionsAsModel
 * @return The missions in a model
 */
QStandardItemModel* DatabaseManager::getMissionsAsModel()
{
    QStandardItemModel* model = new QStandardItemModel;
    if (db.open()) {
        QSqlQuery query(db);
        QString sqlQuery = "SELECT * FROM mission ORDER BY departure_time DESC";
        if (query.exec(sqlQuery)) {
            while( query.next() )
            {
                model->appendRow(new QStandardItem(query.value(1).toString()));
            }
        } else {
            qDebug() << "[getMissionsAsModel()] SELECT failed !" << query.lastError();
        }
    } else {
        qDebug() << "DB not open !";
    }
    db.close();
    return model;
}

/**
 * Get the data types available for a mission
 * @brief DatabaseManager::getDataForMissionsAsModel
 * @param missionName The name of the mission
 * @return
 */
QStandardItemModel* DatabaseManager::getDataForMissionsAsModel(QString missionName)
{
    QStandardItemModel* model = new QStandardItemModel;
    if (db.open()) {
        QSqlQuery query(db);
        QString sqlQuery = "SELECT dfm.* FROM dataformission AS dfm, mission WHERE dfm.mission_id = mission.id AND mission.mission_id = '"+ missionName +"'";
        if (query.exec(sqlQuery)) {
            while( query.next() )
            {
                model->appendRow(new QStandardItem(query.value(2).toString()));
            }
        } else {
            qDebug() << "[getDataForMissionsAsModel()] SELECT failed !" << query.lastError();
        }
    } else {
        qDebug() << "DB not open !";
    }
    db.close();
    if (model->rowCount() == 0)
    {
        model->appendRow(new QStandardItem("No data"));
    }
    return model;
}

/**
 * Get the id of the current mission
 * @brief DatabaseManager::getCurrentMissionId
 * @return The id of the mission
 */
int DatabaseManager::getCurrentMissionId()
{
    int mId = 0;
    if (db.open()) {
        QSqlQuery query(db);
        QString sqlQuery = "SELECT id FROM mission WHERE mission_id = '"+currentMission+"'";
        if (query.exec(sqlQuery)) {
            while( query.next() )
            {
                mId = query.value(0).toInt();
            }
        } else {
            qDebug() << "[getCurrentMissionId()] SELECT failed !" << query.lastError();
        }
    } else {
        qDebug() << "DB not open !";
    }
    db.close();
    return mId;
}

/**
 * Associate datatype to mission for all sensors that are marked 'record'
 * @brief DatabaseManager::setDatatypesForCurrentMission
 * @return True if success
 */
bool DatabaseManager::setDatatypesForCurrentMission()
{
    qDebug() << "setDatatypesForCurrentMission()";
    bool res = true;
    QList<Sensor*> sensors = SensorConfig::instance()->getSensorsRecorded();
    foreach (Sensor* s, sensors) {
        qDebug() << "Associate: " + s->type()->getName();
        if (!addDatatypeForCurrentMission(s->type()->getName()))
        {
            res = false;
        }
    }
    return res;
}

/**
 * Associate a datatype to the current mission (persisted in DB)
 * @brief DatabaseManager::addDataForCurrentMission
 * @param datatype The datatype to associate
 * @return True if success
 */
bool DatabaseManager::addDatatypeForCurrentMission(QString datatype)
{
    DbTable dfmTable = tables["dataformission"];
    QList<QVariant> values = QList<QVariant>();
    values.append(getCurrentMissionId());
    values.append(datatype);
    return insertValue(dfmTable, values);
}
