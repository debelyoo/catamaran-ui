#include "dataExporter.h"

DataExporter* DataExporter::s_instance = 0;

/** This function is called to create an instance of the class.
    Calling the constructor publicly is not allowed. The constructor
    is private and is only called by this Instance function.
*/
DataExporter* DataExporter::instance()
{
    if (!s_instance)   // Only allow one instance of class to be generated.
    {
        s_instance = new DataExporter();
    }
    return s_instance;
}

void DataExporter::exportData(QString missionName, QList<QString> sensorTypeList)
{
    tempMissionName = missionName;
    tempSensorTypeList = sensorTypeList;
    // send mission first. Once we get the ack, send data
    sendMission();
}

/**
 * Send mission (and sensors) to backend - POST request
 * @brief DataExporter::sendMission
 */
void DataExporter::sendMission()
{
    QPair<int, QJsonDocument> jsonDataWithCode = dbManager->getMissionAsJSON(tempMissionName); // get mission and sensors associated
    if (jsonDataWithCode.first != 0) {
        emit displayInGui("[ERROR] Could not export data. Check debug console for more info.\n");
    } else {
        httpRequester->sendPostRequest(QString("/portal/api/mission"), jsonDataWithCode.second);
        //qDebug() << "Mission [" << tempMissionName << "] has been sent to server !";
        emit displayInGui("Mission [" + tempMissionName + "] has been sent to server !\n");
    }
}

/**
 * Send data to backend - POST request
 * @brief DataExporter::sendData
 * @param missionIdOnBackend
 */
void DataExporter::sendData(long missionIdOnBackend)
{
    jsChunksToSend = QQueue<QJsonDocument>(); // reset queue
    foreach (QString sensorType, tempSensorTypeList) {
        QList<QJsonObject> jsDataList = dbManager->getDataAsJSON(tempMissionName, sensorType, missionIdOnBackend);
        prepareDataChunk(jsDataList, sensorType);
    }
    sendNextDataJsonChunkInQueue();
}

/**
 * Prepare JSON documents with a maximum number (MAX_DATA_LOG_IN_EXPORT_REQUEST) of data logs,
 * and add them in the "toSend" queue.
 * The JSON documents look like this: {"datatype": "temperature", "items": [...]}
 * @brief DataExporter::prepareDataChunk
 * @param jsDataList The list of data records (in JSON)
 * @param sensorType The type of sensor
 */
void DataExporter::prepareDataChunk(QList<QJsonObject> jsDataList, QString sensorType)
{
    QJsonArray jArr;
    int inc = 0;
    foreach (QJsonObject item, jsDataList) {
        jArr.append(item);
        inc++;
        if (jArr.size() == MAX_DATA_LOG_IN_EXPORT_REQUEST || inc == jsDataList.length()) {
            QJsonObject json;
            json.insert("datatype", sensorType);
            json.insert("items", jArr);
            json.insert("last_chunk", inc == jsDataList.length());
            jsChunksToSend.enqueue(QJsonDocument(json));
            jArr = QJsonArray(); // reset array
        }
    }
}

/**
 * Take the next JSON doc in queue and send it to backend
 * @brief DataExporter::sendNextDataJsonChunkInQueue
 */
void DataExporter::sendNextDataJsonChunkInQueue()
{
    if(!jsChunksToSend.isEmpty())
    {
        QJsonDocument chunk = jsChunksToSend.dequeue();
        httpRequester->sendPostRequest(QString("/portal/api/data"), chunk);
    } else {
        emit displayInGui("Data has been sent to server !\n");
    }
}

void DataExporter::on_requestFinished(QNetworkReply* reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    //qDebug() << "on_requestFinished() " << statusCode << " - " << reply->url().toString();
    if (reply->url().toString().contains("/api/mission")) {
        // POST mission request done
        switch (statusCode) {
        case 200:
        {
            QByteArray bytes = reply->readAll();  // bytes
            QJsonDocument json = QJsonDocument::fromJson(bytes);
            long missionIdOnBackend = (long)json.object().value("mission_id").toDouble(); // need to use toDouble and then cast to long because there is not toLong() function on QJsonValue
            //qDebug() << "mission id: " << missionIdOnBackend;
            sendData(missionIdOnBackend);
            break;
        }
        default:
            // error
            qDebug() << "[ERROR - " << statusCode << "] while exporting mission";
            break;
        }
    } else if(reply->url().toString().contains("/api/data")) {
        // POST data request done
        if (statusCode == 200) {
            sendNextDataJsonChunkInQueue();
        } else {
            qDebug() << "[ERROR - "<< statusCode <<"] while exporting data";
        }
    }
}
