#ifndef DATAEXPORTER_H
#define DATAEXPORTER_H

#include "httpRequester.h"
#include "util/database/databaseManager.h"

class DataExporter : public QObject
{
    Q_OBJECT
    public:
        static DataExporter* instance();
        void exportData(QString missionName, QList<QString> sensorTypeList);

    signals:
        void displayInGui(QString);

    public slots:
        void on_requestFinished(QNetworkReply*);


    private:
        Q_DISABLE_COPY(DataExporter)
        DataExporter() {
            httpRequester = HttpRequester::instance();
            dbManager = DatabaseManager::instance();
            connect(httpRequester, SIGNAL(done(QNetworkReply*)), this, SLOT(on_requestFinished(QNetworkReply*)));
        }
        static DataExporter* s_instance;

        HttpRequester* httpRequester;
        DatabaseManager* dbManager;
        QString tempMissionName;
        QList<QString> tempSensorTypeList;
        QQueue<QJsonDocument> jsChunksToSend;
        static const int MAX_DATA_LOG_IN_EXPORT_REQUEST = 1000;

        void sendMission();
        void sendData(long missionIdOnBackend);
        void prepareDataChunk(QList<QJsonObject> jsDataList, QString sensorType);
        void sendNextDataJsonChunkInQueue();
};

#endif // DATAEXPORTER_H
