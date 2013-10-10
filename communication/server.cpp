#include "server.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <cstdio>
#include <QVector>
#include <QNetworkInterface>
#include <QDir>

Server::Server(QObject *parent) :
    QObject(parent),
    inputStream(NULL),
    consumer(NULL)
{
    sensorConfig = SensorConfig::instance();
    dbManager = DatabaseManager::instance();
    server = new QTcpServer(this);
    //queue = new QQueue<char>();
    connected = false;
    connect(server, SIGNAL(newConnection()), this, SLOT(on_newConnection()));
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
/*
QByteArray Server::prepareConfigMessage()
{
    quint8 command = MessageUtil::Set;
    quint8 addr = 5;
    QByteArray data;
    data.push_back(command);
    data.push_back(converter->intToByteArray(2, 4));
    // add address
    data.push_back(converter->byteArrayForCmdParameterInt(addr));
    // add stream array
    //qDebug() << "send array of "+ QString::number(sensorConfig->getSensors().length()) +" sensors to stream";
    data.push_back(converter->byteArrayForCmdParameterStreamArray(sensorConfig->getSensors()));
    return data;
}
*/
void Server::sendCommandMessage(QByteArray msg)
{
    if (isConnected())
        publisher->sendCommandMessage(msg);
}

void Server::sendMessage(const CRioByteArray &cba)
{
    if (isConnected())
        publisher->sendMessage(cba);
}

bool Server::isConnected()
{
    return connected;
}

