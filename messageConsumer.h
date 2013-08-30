#ifndef MESSAGECONSUMER_H
#define MESSAGECONSUMER_H

#include "byteArrayConverter.h"
#include "sensorConfig.h"
#include "coordinateHelper.h"
#include "fileHelper.h"
#include <QObject>
#include <QQueue>

class MessageConsumer : public QObject
{
    Q_OBJECT
    public:
        explicit MessageConsumer(QObject *parent = 0, QQueue<char> *q = 0);
        void handleMessageData(int address, QVector< QPair<QVariant, DataType::Types> > v, double ts); // TODO - test

    signals:
        void messageParsed(QString);
        void gpsPointReceived(double, double);

    public slots:
        void on_dataReceived();

    private:
        QQueue<char> *queue;
        bool consuming;
        int waitingData;
        ByteArrayConverter *converter; // singleton
        SensorConfig *sensorConfig; // singleton
        CoordinateHelper* coordinateHelper; // singleton
        FileHelper *fileHelper; // singleton
        void checkQueue(bool checkIfConsuming);
        void readQueue();
        QByteArray readBytes(int nbBytesToRead);
        void parseDataMessage();
        QPair<QVariant, DataType::Types> decodeDataValue();
        double decodeTimestamp();
        void parseCmdMessage();
        void handleGetCommand(int address);

};

#endif // MESSAGECONSUMER_H
