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
    // create sensorconfig table
    cols.clear();
    cols.append(DbColumn("mission_id", SQLite::INTEGER));
    cols.append(DbColumn("config_blob", SQLite::BLOB));
    DbTable sensorConfigTable = DbTable("sensorconfig", cols);
    createTable(TableList::SENSOR_CONFIG, sensorConfigTable);
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

bool DatabaseManager::insertSensorValue(QString sensorAddress, QString sensorType, qint64 unixTs, double value)
{
    //qDebug() << "[DatabaseManager.insertValue()] table: "+table.getName();
    bool res;
    DbTable table = tables[TableList::SENSOR_LOG];
    double ts = (double)unixTs / 1000;
    QList<QVariant> values = QList<QVariant>();
    values.append(currentMissionId);
    values.append(sensorAddress);
    values.append(sensorType);
    values.append(ts);
    values.append(value);
    res = insertRecord(table, values);
    return res;
}


bool DatabaseManager::insertGpsPoint(qint64 unixTs, double lat, double lon, double alt, double heading)
{
    //qDebug() << "[DatabaseManager.insertValue()] table: "+table.getName();
    bool res;
    DbTable table = tables[TableList::GPS_LOG];
    double ts = (double)unixTs / 1000;
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

bool DatabaseManager::insertSensorConfigBlob(QByteArray blob)
{
    bool res;
    qint64 missionId = 0; // TODO - change fix mission id
    // remove blob record if it already exists for this mission
    removeSensorConfigBlobForMission(missionId);
    // insert new blob
    DbTable table = tables[TableList::SENSOR_CONFIG];
    QList<QVariant> values = QList<QVariant>();
    values.append(missionId);
    values.append(blob);
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
            //qDebug() << "table creation succeeded ! "+ table.getName();
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

bool DatabaseManager::createNewMission()
{
    bool b = insertMission();
    FileHelper::instance()->createLogFiles(true);
    return b;
}

bool DatabaseManager::insertMission()
{
    bool res;
    // mission_id - departure time
    QList<QVariant> values;
    QDateTime dateTime = QDateTime::currentDateTime();
    QString tzOffset = TimeHelper::getTimeZoneOffset();
    QString timeZoneStr = "GMT"+tzOffset; // set timezone id (e.g. GMT+1, GMT+2, GMT+9)
    QString missionName = "mission_"+dateTime.toString("yyyyMMdd_hh:mm:ss");
    double ts = (double)dateTime.currentMSecsSinceEpoch() / 1000;
    values.append(missionName);
    values.append(ts);
    values.append(timeZoneStr);
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
 * Remove a mission from the SQLite database (with the related data)
 * @brief DatabaseManager::removeMission
 * @param missionName
 * @return
 */
bool DatabaseManager::removeMission(QString missionName)
{
    bool res = false;
    QList<QJsonObject> jsDataList;
    qint64 missionId = getMission(missionName).getId();
    DbTable gpsTable = tables[TableList::GPS_LOG];
    DbTable sensorLogTable = tables[TableList::SENSOR_LOG];
    DbTable missionTable = tables[TableList::MISSION];
    QString sqlQueryDeleteMission = "DELETE FROM "+ missionTable.getName() +" WHERE id = "+ QString::number(missionId)+";";
    QString sqlQueryDeleteGpsLog = "DELETE FROM "+ gpsTable.getName() +" WHERE mission_id = "+ QString::number(missionId)+";";
    QString sqlQueryDeleteSensorLog = "DELETE FROM "+ sensorLogTable.getName() +" WHERE mission_id = "+ QString::number(missionId)+";";
    if (db.open()) {
        QSqlQuery query(db);
        bool b1 = query.exec(sqlQueryDeleteMission);
        bool b2 = query.exec(sqlQueryDeleteGpsLog);
        bool b3 = query.exec(sqlQueryDeleteSensorLog);
        res = b1 && b2 && b3;
        if (!res) {
            qDebug() << "delete failed !" << query.lastError();
        }
    } else {
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
        QString sqlQuery = "SELECT * FROM sensorlog WHERE mission_id = "+ currentMissionId;
        sqlQuery += " AND sensor_address = '"+ s->address()+"'";
        sqlQuery += " AND timestamp > "+ QString::number(fromTs);
        sqlQuery += " ORDER BY timestamp";// LIMIT 100";
        sqlQuery += ";";
        //qDebug() << "SQL query: " << sqlQuery;
        if (query.exec(sqlQuery)) {
            while( query.next() )
            {
                logTimes.push_back(query.value( 4 ).toDouble()); // times are in seconds to fit with QCustomPlot
                logValues.push_back(query.value( 5 ).toDouble());
            }
        } else {
            qDebug() << "select failed !" << query.lastError();
        }
    } else {
        qDebug() << "DB not open !";
    }
    db.close();
    /*
    if (logValues.size() > 0)
    {
        qDebug() << "getData() - values: "+ QString::number(logValues.size())+" for sensor address: "+ s->address();
        //qDebug() << "getData() - last time: "+ QString::number(logTimes.last(), 'f', 10)+" last value: "+ QString::number(logValues.last());
    }*/
    QPair< QVector<double>, QVector<double> > data = QPair< QVector<double>, QVector<double> >(logTimes, logValues);
    return data;
}

/**
 * Get data as JSON. Each table row is formatted as a JSON object, and appended to a JSON array
 * @brief DatabaseManager::getDataAsJSON
 * @param missionName The name of the mission
 * @param sensorType The sensor type
 * @param missionIdOnBackend The id of the mission (in backend DB)
 * @return A JSON array with the data records - {"datatype":"gps", "items": [...]}
 */
QList<QJsonObject> DatabaseManager::getDataAsJSON(QString missionName, QString sensorType, long missionIdOnBackend)
{
    QList<QJsonObject> jsDataList;
    qint64 missionId = getMission(missionName).getId();
    DbTable table;
    QString sqlQuery;
    if (sensorType == "gps") {
        table = tables[TableList::GPS_LOG];
        sqlQuery = "SELECT * FROM "+ table.getName() +" WHERE mission_id = "+ QString::number(missionId);
        sqlQuery += " ORDER BY timestamp;"; // LIMIT 100";
    } else {
        table = tables[TableList::SENSOR_LOG];
        sqlQuery = "SELECT * FROM "+ table.getName() +" WHERE mission_id = "+ QString::number(missionId)+" AND sensor_type = '"+ sensorType +"'";
        sqlQuery += " ORDER BY timestamp;"; // LIMIT 100";
    }
    if (db.open()) {
        QSqlQuery query(db);
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
                        switch (col.getType()) {
                        case SQLite::TEXT:
                            jObj.insert(col.getName(), query.value(i+1).toString()); // i+1 because first field is id, which is not in the table columns since it is standard for all tables
                            break;
                        case SQLite::INTEGER:
                            jObj.insert(col.getName(), query.value(i+1).toInt()); // i+1 because first field is id, which is not in the table columns since it is standard for all tables
                            break;
                        case SQLite::REAL:
                            jObj.insert(col.getName(), query.value(i+1).toDouble()); // i+1 because first field is id, which is not in the table columns since it is standard for all tables
                            break;
                        default:
                            break;
                        }
                    }
                }
                jsDataList.append(jObj);
            }
        } else {
            qDebug() << "[getDataAsJSON()] select failed !" << query.lastError();
        }
    } else {
        qDebug() << "DB not open !";
    }
    db.close();
    return jsDataList;
}

/**
 * Get a mission as JSON object
 * @brief DatabaseManager::getMissionAsJSON
 * @param missionName The name of the mission
 * @return A JSON object with the mission details - {"departure_time": "1382190049.321", "timezone": "GMT+2", "vehicle": "catamaran"}
 */
QPair<int, QJsonDocument> DatabaseManager::getMissionAsJSON(QString missionName)
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
                QJsonArray jsSensorArray;
                // get sensors
                QPair<int, QList<Sensor*> > res = getSensorsForMission(missionName);
                if (res.first != 0) {
                    // error occurred
                    return QPair<int, QJsonDocument>(1, QJsonDocument());
                }
                QList<Sensor*> sensors = res.second;
                //sensors.append(SensorConfig::instance()->getSensor("48")); // add GPS sensor to list
                foreach (Sensor* s, sensors) {
                    QJsonObject jsDev;
                    jsDev.insert("address", s->address());
                    jsDev.insert("name", s->name());
                    jsDev.insert("datatype", s->type()->name());
                    jsSensorArray.append(jsDev);
                }
                json.insert("devices", jsSensorArray);
            }
        } else {
            qDebug() << "[getMissionAsJSON()] select failed !" << query.lastError();
        }
    } else {
        qDebug() << "DB not open !";
    }
    db.close();
    //qDebug() << "Mission JSON: " << json;
    return QPair<int, QJsonDocument>(0, QJsonDocument(json));
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
    //qDebug() << "getDataForMissionsAsModel() - " << missionName;
    qint64 missionId = getMission(missionName).getId();
    QStandardItemModel* model = new QStandardItemModel;
    if (db.open()) {
        QSqlQuery query(db);
        QString sqlQuery;
        sqlQuery = "SELECT DISTINCT mission_id FROM gpslog WHERE mission_id = "+ QString::number(missionId);
        if (query.exec(sqlQuery)) {
            while( query.next() )
            {
                model->appendRow(new QStandardItem("gps"));
            }
        } else {
            qDebug() << "[getDataForMissionsAsModel() - gpslog] SELECT failed !" << query.lastError();
        }
        sqlQuery = "SELECT DISTINCT sensor_type FROM sensorlog WHERE mission_id = "+ QString::number(missionId);
        if (query.exec(sqlQuery)) {
            while( query.next() )
            {
                model->appendRow(new QStandardItem(query.value(0).toString()));
            }
        } else {
            qDebug() << "[getDataForMissionsAsModel() - sensorlog] SELECT failed !" << query.lastError();
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
 * Get the sensors associated to a mission
 * @brief DatabaseManager::getSensorsForMission
 * @param missionName The name of the mission
 * @return A pair with error code and list of sensors
 */
QPair<int, QList<Sensor*> > DatabaseManager::getSensorsForMission(QString missionName)
{
    QList<Sensor*> sensors;
    qint64 missionId = getMission(missionName).getId();
     if (db.open()) {
         QSqlQuery query(db);
         QString sqlQuery = "SELECT DISTINCT sensor_address FROM sensorlog WHERE mission_id = " + QString::number(missionId);
         if (query.exec(sqlQuery)) {
             while( query.next() )
             {
                 QString sensorAddress = query.value(0).toString();
                 Sensor* s = SensorConfig::instance()->getSensor(sensorAddress);
                 if (s == NULL) {
                     qDebug() << "[getSensorsForMission()] Sensor (" << sensorAddress << ") does not exist in SensorConfig !";
                     return QPair<int, QList<Sensor*> >(1, QList<Sensor*>());
                 }
                 sensors.append(s);
             }
         } else {
             qDebug() << "[getSensorsForMission()] SELECT failed !" << query.lastError();
         }
     } else {
         qDebug() << "DB not open !";
     }
     db.close();
     return QPair<int, QList<Sensor*> >(0, sensors);
}

QByteArray DatabaseManager::getSensorConfigBlob(qint64 mId)
{
    QByteArray blob;
    /*qint64 missionId;
    if (mId == 0) {
        missionId = currentMissionId;
    } else {
        missionId = mId;
    }*/
    if (db.open()) {
         QSqlQuery query(db);
         QString sqlQuery = "SELECT config_blob FROM sensorconfig WHERE mission_id = " + QString::number(mId);
         if (query.exec(sqlQuery)) {
             while( query.next() )
             {
                blob = query.value(0).toByteArray();
             }
         } else {
             qDebug() << "[getSensorConfigBlob()] SELECT failed !" << query.lastError();
         }
    } else {
        qDebug() << "DB not open !";
    }
    db.close();
    return blob;
}

QString DatabaseManager::getCurrentMissionName()
{
    return currentMissionName;
}

bool DatabaseManager::removeSensorConfigBlobForMission(qint64 missionId)
{
    bool res = false;
    DbTable sensorConfigTable = tables[TableList::SENSOR_CONFIG];
    QString sqlQueryDelete = "DELETE FROM "+ sensorConfigTable.getName() +" WHERE mission_id = "+ QString::number(missionId)+";";
    if (db.open()) {
        QSqlQuery query(db);
        res = query.exec(sqlQueryDelete);
        if (!res) {
            qDebug() << "delete failed !" << query.lastError();
        }
    } else {
        qDebug() << "DB not open !";
    }
    db.close();
    return res;
}

/// TEST ONLY
void DatabaseManager::insertSampleData()
{
    static int MAX_VALUE = 30;
    qint64 now = QDateTime::currentDateTime().toTime_t() * 1000;
    insertGpsPoint(now, 46.5171, 6.5817, 373.24, 120.0);
    for (int i = 0; i < 100; i++) {
        double r = (double)rand() / (double)MAX_VALUE;
        insertSensorValue("96", "PT100", now + i * 1000, r);
    }
}
