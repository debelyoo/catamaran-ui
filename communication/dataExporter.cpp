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

void DataExporter::exportData(QString missionName, QString dataType)
{
    tempMissionName = missionName;
    tempDataType = dataType;
    // send mission first. Once we get the ack, send data
    sendMission();
}

void DataExporter::sendMission()
{
    QJsonDocument jsonData = dbManager->getMissionAsJSON(tempMissionName);
    httpRequester->sendPostRequest(QString("/portal/api/mission"), jsonData);
}

void DataExporter::sendData(long missionIdOnBackend)
{
    //const SensorType* st = SensorTypeManager::instance()->type(datatype);
    const SensorType* st = new SensorType(tempDataType, "gpslog", 0); // TODO - temporary test
    QJsonDocument jsonData = dbManager->getDataAsJSON(tempMissionName, st, missionIdOnBackend);
    httpRequester->sendPostRequest(QString("/portal/api/data"), jsonData);
}

void DataExporter::on_requestFinished(QNetworkReply* reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    //qDebug() << "on_requestFinished() " << statusCode << " - " << reply->url().toString();
    if (reply->url().toString().contains("/ping")) {
        emit pingRequestDone(statusCode);
    } else if (reply->url().toString().contains("/api/mission")) {
        // POST mission request done
        switch (statusCode) {
        case 200:
        {
            QByteArray bytes = reply->readAll();  // bytes
            QJsonDocument json = QJsonDocument::fromJson(bytes);
            long missionIdOnBackend = (long)json.object().value("mission_id").toDouble(); // need to use toDouble and then cast to long because there is not toLong() function on QJsonValue
            //qDebug() << missionId;
            sendData(missionIdOnBackend);
            break;
        }
        default:
            // error
            break;
        }
    } else if(reply->url().toString().contains("/api/data")) {
        // POST data request done
    }
}

void DataExporter::sendPingRequest()
{
    httpRequester->sendPingRequest();
}
