#ifndef DATAEXPORTER_H
#define DATAEXPORTER_H

#include "httpRequester.h"
#include "util/database/databaseManager.h"

class DataExporter : public QObject
{
    Q_OBJECT
    public:
        static DataExporter* instance();
        void exportData(QString missionName, QString dataType);

    signals:
        void pingRequestDone(int statusCode);

    public slots:
        void sendPingRequest();
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
        QString tempDataType;

        void sendMission();
        void sendData(long missionIdOnBackend);
};

#endif // DATAEXPORTER_H
