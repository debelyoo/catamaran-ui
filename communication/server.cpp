#include "server.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <cstdio>
#include <QVector>
#include <QNetworkInterface>
#include <QDir>

Server *Server::s_instance = NULL;

Server::Server() :
    QObject(),
    inputStream(NULL),
    consumer(NULL),
    m_commandQueue(),
    m_dataQueue(),
    m_queuesEnabled(false)
{

    sensorConfig = SensorConfig::instance();
    dbManager = DatabaseManager::instance();
    server = new QTcpServer(this);
    //queue = new QQueue<char>();
    connected = false;
    connect(server, SIGNAL(newConnection()), this, SLOT(on_newConnection()));

    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(on_timerTimeout()));
}

Server *Server::instance()
{
    if(!s_instance){
        s_instance = new Server();
    }
    return s_instance;
}

void Server::listen()
{
    int port = 5555;
    QHostAddress ipAddress;
    foreach (QHostAddress address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
        {
            ipAddress = address;
        }
    }
    QString msg = "Listening on " + ipAddress.toString() +":"+QString::number(port)+"\n";
    emit displayInGui(msg);
    server->listen(QHostAddress(ipAddress), port);


}

void Server::on_newConnection()
{
    socket = server->nextPendingConnection();
    if(socket->state() == QTcpSocket::ConnectedState)
    {
        emit displayInGui("New connection established.\n");
        connected = true;
    }
    //inputStream->setDevice(socket);
    inputStream = new CRioDataStream(socket);
    consumer = new MessageConsumer(this, inputStream);
    publisher = new MessagePublisher(this, socket);

    connect(this, SIGNAL(dataReceived()), consumer, SLOT(on_dataReceived())); // notify the consumer that some data has arrived
    /// signals to be relayed to GUI
    connect(consumer, SIGNAL(messageParsed(QString)), this, SLOT(on_messageParsed(QString)));
    connect(consumer, SIGNAL(gpsPointReceived(double,double)), this, SLOT(on_gpsPointReceived(double,double)));

    connect(socket, SIGNAL(disconnected()), this, SLOT(on_disconnected()));
    //connect(socket, SIGNAL(readyRead()), this, SLOT(on_readyRead()));
    connect(socket, SIGNAL(readyRead()), this, SIGNAL(dataReceived()));
    emit newConnection();
}
/*
void Server::on_readyRead()
{
    while(socket->bytesAvailable() > 0)
    {
        //QByteArray ba = socket->readLine();
        QByteArray ba = socket->read(BYTE_CHUNK_SIZE_TO_READ);
        if(strcmp(ba.constData(), "!exit\n") == 0)
        {
            socket->disconnectFromHost();
            break;
        }
        //printf(">> %s", ba.constData());
        //fflush(stdout);
        //printf("bytearray size >> %d\n", ba.length());

        for (int i=0; i < ba.length(); i++) {
            char c = ba.at(i);
            queue->enqueue(c);
        }
        //printf("queue size >> %d \n", queue->size());

        emit dataReceived();
    }
}
*/
void Server::on_disconnected()
{
    //printf("Connection disconnected.\n");
    emit displayInGui("Connection disconnected.\n");
    connected = false;
    fflush(stdout);
    disconnect(socket, SIGNAL(disconnected()));
    disconnect(socket, SIGNAL(readyRead()));
    socket->deleteLater();
}

/**
 * Relay the "message parsed" signal to the GUI
 * @brief Server::on_messageParsed
 * @param msg
 */
void Server::on_messageParsed(QString msg)
{
    emit displayInGui(msg);
}

/**
 * Relay the "gpsPointReceived" signal to the GUI
 * @brief Server::on_gpsPointReceived
 * @param x
 * @param y
 */
void Server::on_gpsPointReceived(double x, double y)
{
    emit gpsPointReceived(x, y);
}

void Server::on_timerTimeout()
{
    if (isConnected()){
        while(!m_dataQueue.isEmpty()){
            sendMessage(CRioByteArray(m_dataQueue.dequeue()));
        }
        while(!m_dataQueue.isEmpty()){
            sendMessage(CRioByteArray(m_commandQueue.dequeue()));
        }
    }
}

bool Server::sendMessage(const CRioByteArray &cba)
{
    if (isConnected()){
        publisher->sendMessage(cba);
        return true;
    }
    return false;
}

bool Server::sendMessage(const CRioCommand &cmd)
{
    if (isConnected()){
        publisher->sendMessage(CRioByteArray(cmd));
        return true;
    }
    //m_queuesEnabled?m_commandQueue.append(cmd):void();
    return false;
}

bool Server::sendMessage(const CRioData &data)
{
    if (isConnected()){
        publisher->sendMessage(CRioByteArray(data));
        return true;
    }
    m_queuesEnabled?m_dataQueue.append(data):void();
    return false;
}

void Server::enableQueues()
{
    m_queuesEnabled = true;
    m_timer->start();
}

void Server::disableQueues()
{
    m_queuesEnabled = false;
    m_timer->stop();
}

bool Server::queueEnabled() const
{
    return m_queuesEnabled;
}

bool Server::isConnected() const
{
    return connected;
}

