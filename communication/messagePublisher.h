#ifndef MESSAGEPUBLISHER_H
#define MESSAGEPUBLISHER_H

#include "byteArrayConverter.h"
#include <QObject>
#include <QTcpSocket>

class MessagePublisher : public QObject
{
    Q_OBJECT
    public:
        explicit MessagePublisher(QObject *parent = 0, QTcpSocket* socket = 0);
    signals:

    public slots:
        void sendCommandMessage(int, QByteArray);

    private:
        QTcpSocket* socket;
        ByteArrayConverter* converter;

};

#endif // MESSAGEPUBLISHER_H
