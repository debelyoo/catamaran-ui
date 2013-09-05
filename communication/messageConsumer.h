#ifndef MESSAGECONSUMER_H
#define MESSAGECONSUMER_H

#include "util/byteArrayConverter.h"
#include "sensorConfig.h"
#include "util/coordinateHelper.h"
#include "util/fileHelper.h"
#include "util/timeHelper.h"
#include "util/databaseManager.h"
#include "dataObject.h"
#include <QObject>
#include <QQueue>

class MessageConsumer : public QObject
{
    Q_OBJECT
    public:
        explicit MessageConsumer(QObject *parent = 0, QQueue<char> *q = 0);
        void handleMessageData(DataObject* dataObj);

    signals:
        void messageParsed(QString);
        void gpsPointReceived(double, double);

    public slots:
        void on_dataReceived();

    private:
        QQueue<char> *queue;
        bool consuming;
        int waitingData;
        ByteArrayConverter *converter;
        SensorConfig *sensorConfig;
        CoordinateHelper* coordinateHelper;
        FileHelper *fileHelper;
        DatabaseManager* dbManager;
        void checkQueue(bool checkIfConsuming);
        void readQueue();
        QByteArray readBytes(int nbBytesToRead);
        void parseDataMessage();
        QPair<QVariant, DataType::Types> decodeDataValue();
        qint64 decodeTimestamp();
        qint64 decodeTimestamp2();
        void parseCmdMessage();
        void handleGetCommand(int address);

};

#endif // MESSAGECONSUMER_H
