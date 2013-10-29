#ifndef HTTPREQUESTER_H
#define HTTPREQUESTER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

class HttpRequester : public QObject
{
    Q_OBJECT
    public:
        static HttpRequester* instance();
        void setBackendAddress(QString address);
        void sendPostRequest(QString urlPath, QJsonDocument jObj);
        void getSensorTypes();

    signals:
        void pingRequestDone(int statusCode);
        void sensorTypeRequestDone(int statusCode, QList<QString> sensorTypes);
        void done(QNetworkReply*);

    public slots:
        void sendPingRequest();
        void on_requestFinished(QNetworkReply*);


    private:
        Q_DISABLE_COPY(HttpRequester)
        HttpRequester() {
            networkManager = new QNetworkAccessManager();
            backendAddress = "localhost"; // default
            connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(on_requestFinished(QNetworkReply*)));
        }

        static HttpRequester* s_instance;
        QNetworkAccessManager* networkManager;
        QString backendAddress;
};

#endif // HTTPREQUESTER_H
