#include "server.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <cstdio>
#include <QVector>
#include <QNetworkInterface>

Server::Server(QObject *parent) :
QObject(parent)
{
    sensorConfig = SensorConfig::instance();
    server = new QTcpServer(this);
    queue = new QQueue<char>();
    consumer = new MessageConsumer(this, queue);
    connected = false;
    connect(server, SIGNAL(newConnection()), this, SLOT(on_newConnection()));
    connect(this, SIGNAL(dataReceived()), consumer, SLOT(on_dataReceived())); // notify the consumer that some data has arrived
    /// signals to be relayed to GUI
    connect(consumer, SIGNAL(messageParsed(QString)), this, SLOT(on_messageParsed(QString)));
    connect(consumer, SIGNAL(gpsPointReceived(double,double)), this, SLOT(on_gpsPointReceived(double,double)));
}

void Server::listen()
{
    int port = 5555;
    QHostAddress ipAddress;
    foreach (QHostAddress address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
        {
            QString msg = "Listening on " + address.toString() +":"+QString::number(port)+"\n";
            emit displayInGui(msg);
        }
    }
    server->listen(QHostAddress(ipAddress), port);
}

void Server::on_newConnection()
{
    socket = server->nextPendingConnection();
    if(socket->state() == QTcpSocket::ConnectedState)
    {
        printf("New connection established.\n");
        fflush(stdout);
        emit displayInGui("New connection established.\n");
        connected = true;
    }
    publisher = new MessagePublisher(this, socket);
    connect(socket, SIGNAL(disconnected()), this, SLOT(on_disconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(on_readyRead()));
}

void Server::on_readyRead()
{
    //printf("--> on_readyRead()\n");
    //fflush(stdout);
    //while(socket->canReadLine())
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
        //fflush(stdout);

        for (int i=0; i < ba.length(); i++) {
            char c = ba.at(i);
            queue->enqueue(c);
        }
        //printf("queue size >> %d \n", queue->size());
        //fflush(stdout);

        emit dataReceived();
    }
}

void Server::on_disconnected()
{
    printf("Connection disconnected.\n");
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

void Server::sendCommandMessage(int command, QByteArray msg)
{
    if (isConnected())
        publisher->sendCommandMessage(command, msg);
}

bool Server::isConnected()
{
    return connected;
}

