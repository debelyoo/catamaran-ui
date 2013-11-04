#ifndef MESSAGEPUBLISHER_H
#define MESSAGEPUBLISHER_H

#include <QObject>
#include <QTcpSocket>
#include <QSerialPort>
#include "util/criobytearray.h"

class MessagePublisher : public QObject
{
    Q_OBJECT
    public:
        explicit MessagePublisher(QObject *parent = 0, QTcpSocket* socket = 0);
        void sendMessage(QSerialPort* serialPort, QByteArray ba);

    signals:

    public slots:
        //void sendCommandMessage(QByteArray data);
        void sendMessage(const CRioByteArray &cba);

    private:
        QTcpSocket* socket;

};

#endif // MESSAGEPUBLISHER_H
