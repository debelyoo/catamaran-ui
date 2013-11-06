#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QQueue>
#include "messageConsumer.h"
#include "messagePublisher.h"
#include "util/criobytearray.h"
#include "util/criodatastream.h"

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
    static Server *instance();
    void listen();
    bool isConnected() const;
    //QByteArray prepareConfigMessage();
    bool sendMessage(const CRioByteArray &cba);
    bool sendMessage(const CRioCommand &cmd);
    bool sendMessage(const CRioData &data);
    bool sendMessageToVirtualSerialPort(QVariant strData);

    void enableQueues();
    void disableQueues();
    bool queueEnabled() const;

    MessageConsumer *consumer() const;

signals:
    void displayInGui(QString);
    void dataReceived();
    void gpsPointReceived(double, double);
    void newConnection();
    void connectionLost();

public slots:
    void on_newConnection();
    //void on_readyRead();
    void on_disconnected();
    void on_messageParsed(QString);
    void on_gpsPointReceived(double, double);
    void on_timerTimeout();

private:
    Q_DISABLE_COPY(Server)
    static Server * s_instance;
    explicit Server();
    CRioDataStream *inputStream;
    QTcpServer* server;
    QTcpSocket* socket;
    MessageConsumer* m_consumer;
    MessagePublisher* publisher;
    DatabaseManager* dbManager;
    SensorConfig* sensorConfig; // singleton
    static const int BYTE_CHUNK_SIZE_TO_READ = 32; // in bytes
    bool connected;
    QQueue<CRioCommand> m_commandQueue;
    QQueue<CRioData> m_dataQueue;
    QTimer *m_timer;
    bool m_queuesEnabled;
};

#endif // SERVER_H
