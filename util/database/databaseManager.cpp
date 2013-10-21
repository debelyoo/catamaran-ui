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
    cols.append(DbColumn("name", SQLite::TEXT));
    cols.append(DbColumn("departure_time", SQLite::REAL));
    cols.append(DbColumn("timezone", SQLite::TEXT));
    cols.append(DbColumn("vehicle", SQLite::TEXT));
    DbTable missionTable = DbTable("mission", cols);
    createTable(TableList::MISSION, missionTable);
    // create dataformission table
    cols.clear();
    cols.append(DbColumn("mission_id", SQLite::INTEGER));
    cols.append(DbColumn("datatype", SQLite::TEXT));
    DbTable dfmTable = DbTable("dataformission", cols);
    createTable(TableList::DATA_FOR_MISSION, dfmTable);
    // create gpslog table
    cols.clear();
    cols.append(DbColumn("mission_id", SQLite::INTEGER));
    cols.append(DbColumn("timestamp", SQLite::REAL));
    cols.append(DbColumn("latitude", SQLite::REAL));
    cols.append(DbColumn("longitude", SQLite::REAL));
    cols.append(DbColumn("altitude", SQLite::REAL));
    cols.append(DbColumn("heading", SQLite::REAL));
    DbTable gpsTable = DbTable("gpslog", cols);
    createTable(TableList::GPS_LOG, gpsTable);
    // create sensorlog table
    cols.clear();
    cols.append(DbColumn("mission_id", SQLite::INTEGER));
    cols.append(DbColumn("sensor_address", SQLite::TEXT));
    cols.append(DbColumn("sensor_type", SQLite::TEXT));
    cols.append(DbColumn("timestamp", SQLite::REAL));
    cols.append(DbColumn("value", SQLite::REAL));
    DbTable deviceLogTable = DbTable("sensorlog", cols);
    createTable(TableList::SENSOR_LOG, deviceLogTable);
}

bool DatabaseManager::insertRecord(DbTable table, QList<QVariant> values)
{
    //qDebug() << "[DatabaseManager.insertRecord()] table: "+table.getName();
    bool res;
    if (table.getColumns().length() != values.length()) {
        qDebug() << "[DatabaseManager.insertValue()] [ERROR] nb of columns and values do not match ! [table: "+ table.getName() +", cols: "+table.getColumns().length()+", vals: "+values.length()+"]";
        return false;
    }
    QString queryStr = buildInsertQuery(table);
    //qDebug() << queryStr;
    if (db.open()) {
        QSqlQuery query(db);
        query.prepare(queryStr);
        for (int i = 0; i < table.getColumns().length(); i++) {
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

bool DatabaseManager::insertSensorValue(QString sensorAddress, QString sensorType, double ts, double value)
{
    //qDebug() << "[DatabaseManager.insertValue()] table: "+table.getName();
    bool res;
    DbTable table = tables[TableList::SENSOR_LOG];
    QList<QVariant> values = QList<QVariant>();
    values.append(currentMissionId);
    values.append(sensorAddress);
    values.append(sensorType);
    values.append(ts);
    values.append(value);
    res = insertRecord(table, values);
    return res;
}


bool DatabaseManager::insertGpsPoint(double ts, double lat, double lon, double alt, double heading)
{
    //qDebug() << "[DatabaseManager.insertValue()] table: "+table.getName();
    bool res;
    DbTable table = tables[TableList::GPS_LOG];
    QList<QVariant> values = QList<QVariant>();
    values.append(currentMissionId);
    values.append(ts);
    values.append(lat);
    values.append(lon);
    values.append(alt);
    values.append(heading);
    res = insertRecord(table, values);
    return res;
}

/**
 * Create a table in SQLite database
 * @brief DatabaseManager::createTable
 * @param tableId The id of the table (enum)
 * @param table The table to create
 * @return True if success
 */
bool DatabaseManager::createTable(TableList::Tables tableId, DbTable table)
{
    bool res;
    QString queryStr = buildCreateQuery(table);
    if (db.open()) {
        QSqlQuery query(db);
        if (query.exec(queryStr)) {
            tables.insert(tableId, table);
            res = true;
            qDebug() << "table creation succeeded ! "+ table.getName();
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

bool DatabaseManager::insertMission()
{
    bool res;
    // mission_id - departure time
    QList<QVariant> values;
    QDateTime dateTime = QDateTime::currentDateTime();
    QString missionName = "mission_"+dateTime.toString("yyyyMMdd_hh:mm:ss");;
    double ts = (double)dateTime.currentMSecsSinceEpoch() / 1000;
    values.append(missionName);
    values.append(ts);
    values.append("GMT+2"); // default timezone
    values.append("catamaran");
    boolean b = insertRecord(tables[TableList::MISSION], values);
    if (b) {
        currentMissionId = getMission(missionName).getId();
        currentMissionName = missionName;
        res = b;
    } else {
        res = false;
    }
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
 * Get data as JSON. Each table row is formatted as a JSON object, and appended to a JSON array
 * @brief DatabaseManager::getDataAsJSON
 * @param missionName The name of the mission
 * @param st The sensor type
 * @param missionIdOnBackend The id of the mission (in backend DB)
 * @return A JSON array with the data records - {"datatype":"gps", "items": [...]}
 */
QJsonDocument DatabaseManager::getDataAsJSON(QString missionName, QString sensorType, long missionIdOnBackend)
{
    QJsonObject json;
    json.insert("datatype", sensorType);
    QJsonArray jArr;
    qint64 missionId = getMission(missionName).getId();
    //const SensorType* st = SensorTypeManager::instance()->type(datatype);
    DbTable table = tables[TableList::SENSOR_LOG];
    if (db.open()) {
        QSqlQuery query(db);
        QString sqlQuery = "SELECT * FROM "+ table.getName() +" WHERE mission_id = "+ QString::number(missionId);
        sqlQuery += " ORDER BY timestamp";// LIMIT 100";
        sqlQuery += ";";
        //qDebug() << "SQL query: " << sqlQuery;
        if (query.exec(sqlQuery)) {
            while( query.next() )
            {
                QJsonObject jObj;
                for(int i = 0; i < table.getColumns().length(); i++) {
                    DbColumn col = table.getColumns().at(i);
                    if (col.getName() == "mission_id") {
                        jObj.insert("mission_id", (double)missionIdOnBackend); // need cast to double for QJsonValue
                    } else {
                        jObj.insert(col.getName(), query.value(i+1).toInt()); // i+1 because first field is id, which is not in the table columns since it is standard for all tables
                    }
                }
                jArr.append(jObj);
            }
        } else {
            qDebug() << "[getDataAsJSON()] select failed !" << query.lastError();
        }
    } else {
        qDebug() << "DB not open !";
    }
    db.close();
    json.insert("items", jArr);
    return QJsonDocument(json);
}

/**
 * Get a mission as JSON object
 * @brief DatabaseManager::getMissionAsJSON
 * @param missionName The name of the mission
 * @return A JSON object with the mission details - {"departure_time": "1382190049.321", "timezone": "GMT+2", "vehicle": "catamaran"}
 */
QJsonDocument DatabaseManager::getMissionAsJSON(QString missionName)
{
    QJsonObject json;
    //const SensorType* st = SensorTypeManager::instance()->type(datatype);
    if (db.open()) {
        QSqlQuery query(db);
        QString sqlQuery = "SELECT * FROM mission WHERE name = '"+ missionName+"';"; // mission table: id | name | departure_time | timezone | vehicle
        //qDebug() << "SQL query: " << sqlQuery;
        if (query.exec(sqlQuery)) {
            while( query.next() )
            {
                json.insert("departure_time", query.value(2).toDouble());
                json.insert("timezone", query.value(3).toString());
                json.insert("vehicle", query.value(4).toString());
            }
        } else {
            qDebug() << "[getMissionAsJSON()] select failed !" << query.lastError();
        }
    } else {
        qDebug() << "DB not open !";
    }
    db.close();
    //qDebug() << "Mission JSON: " << json;
    return QJsonDocument(json);
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
 * @return A QStandradItemModel to be used in a QListView
 */
QStandardItemModel* DatabaseManager::getDataForMissionsAsModel(QString missionName)
{
    QStandardItemModel* model = new QStandardItemModel;
    if (db.open()) {
        QSqlQuery query(db);
        QString sqlQuery = "SELECT dfm.* FROM dataformission AS dfm, mission WHERE dfm.mission_id = mission.id AND mission.name = '"+ missionName +"'";
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
 * Get a mission
 * @brief DatabaseManager::getMission
 * @return The mission
 */
Mission DatabaseManager::getMission(QString missionName)
{
   //qint64 mId = 0;
   Mission mission;
    if (db.open()) {
        QSqlQuery query(db);
        QString sqlQuery = "SELECT * FROM mission WHERE name = '"+missionName+"'"; // mission table: id | name | departure_time | timezone | vehicle
        if (query.exec(sqlQuery)) {
            while( query.next() )
            {
                //qDebug() << "mission id: " << query.value(0);
                //mId = query.value(0).toLongLong();
                double depTime = query.value(2).toDouble();
                qint64 depTimeMilli = (qint64)round(depTime * 1000);
                QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(depTimeMilli);
                mission = Mission(query.value(0).toLongLong(), query.value(1).toString(),
                                  dateTime, query.value(3).toString(), query.value(4).toString());
            }
        } else {
            qDebug() << "[getMissionId()] SELECT failed !" << query.lastError();
        }
    } else {
        qDebug() << "DB not open !";
    }
    db.close();
    return mission;
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
    DbTable dfmTable = tables[TableList::DATA_FOR_MISSION];
    QList<QVariant> values = QList<QVariant>();
    values.append(currentMissionId);
    values.append(datatype);
    return insertRecord(dfmTable, values);
}

QString DatabaseManager::getCurrentMissionName()
{
    return currentMissionName;
}

/// TEST ONLY
void DatabaseManager::insertSampleData()
{
    insertGpsPoint(1382093430.5, 46.5171, 6.5817, 373.24, 120.0);
}
