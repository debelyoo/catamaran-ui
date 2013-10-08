#ifndef MESSAGEPUBLISHER_H
#define MESSAGEPUBLISHER_H

#include "util/byteArrayConverter.h"
#include <QObject>
#include <QTcpSocket>
#include "util/criobytearray.h"

class MessagePublisher : public QObject
{
    Q_OBJECT
    public:
        explicit MessagePublisher(QObject *parent = 0, QTcpSocket* socket = 0);
    signals:

    public slots:
        void sendCommandMessage(QByteArray data);
        void sendMessage(const CRioByteArray &cba);

    private:
        QTcpSocket* socket;
        ByteArrayConverter* converter;

};

#endif // MESSAGEPUBLISHER_H
