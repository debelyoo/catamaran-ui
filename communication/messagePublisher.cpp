#include "messagePublisher.h"
#include "server.h"

MessagePublisher::MessagePublisher(QObject *parent, QTcpSocket *sk) :
QObject(parent)
{
    socket = sk;
}

/**
 * Send a command message to cRIO
 * @brief MessagePublisher::sendCommandMessage
 * @param data
 */
void MessagePublisher::sendCommandMessage(QByteArray data)
{
    // CMD(uint8) | length (4B) | params (bytes array)
    quint8 msgType = MessageUtil::Cmd;
    QByteArray msgLen = converter->intToByteArray(data.length(), 4);
    QByteArray ba;
    ba.push_back(msgType);
    ba.push_back(msgLen);
    ba.push_back(data);
    int bytesWritten = socket->write(ba);
    bool bytesFlushed = socket->flush();
    char str[128];
    sprintf(str, "Wrote [%d] bytes on wire - flushed: %d", bytesWritten, bytesFlushed);
    //sprintf(str, "Sent value [%d] \n", val);
    qDebug() << str;
    //qDebug() << ba;
}

void MessagePublisher::sendMessage(const CRioByteArray &cba)
{
    int bytesWritten = socket->write(cba.byteArray());
    bool bytesFlushed = socket->flush();
    qDebug() << QString("Wrote [%1] bytes on wire - flushed: %2. ba=").arg(bytesWritten).arg(bytesFlushed) << cba.byteArray().toHex();
}

