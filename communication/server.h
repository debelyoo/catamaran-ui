#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QQueue>
#include "messageConsumer.h"
#include "messagePublisher.h"

class QTcpServer;
class QTcpSocket;

namespace MessageUtil {
    typedef enum {
        Stop = 0,
        Get = 1,
        Set = 2,
        Add = 3,
        Delete = 4
    } Commands;

    typedef enum {
        Data = 0,
        Cmd = 1
    } MessageTypes;
}

class Server : public QObject
{
    Q_OBJECT
    public:
        explicit Server(QObject *parent = 0);
        void listen();
        bool isConnected();
        void sendCommandMessage(QByteArray data);

    signals:
        void displayInGui(QString);
        void dataReceived();
        void gpsPointReceived(double, double);

    public slots:
        void on_newConnection();
        void on_readyRead();
        void on_disconnected();
        void on_messageParsed(QString);
        void on_gpsPointReceived(double, double);

    private:
        QTcpServer* server;
        QTcpSocket* socket;
        MessageConsumer* consumer;
        MessagePublisher* publisher;
        DatabaseManager* dbManager;
        SensorConfig* sensorConfig; // singleton
        QQueue<char> *queue;
        static const int BYTE_CHUNK_SIZE_TO_READ = 32; // in bytes
        bool connected;
};

#endif // SERVER_H
