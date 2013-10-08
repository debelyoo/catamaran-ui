#ifndef MESSAGECONSUMER_H
#define MESSAGECONSUMER_H

#include "util/byteArrayConverter.h"
#include "sensorConfig.h"
#include "util/coordinateHelper.h"
#include "util/fileHelper.h"
#include "util/timeHelper.h"
#include "util/databaseManager.h"
#include "dataObject.h"
#include "idatamessagereceiver.h"
#include "transformation/transformationmanager.h"
#include <QObject>
#include <QQueue>
#include "util/criodatastream.h"

class MessageConsumer : public QObject, public IDataMessageReceiver
{
    Q_OBJECT
    public:
        explicit MessageConsumer(QObject *parent = 0, CRioDataStream* ds = 0);
        void handleMessageData(DataObject dataObj);
        ~MessageConsumer();

    signals:
        void messageParsed(QString);
        void gpsPointReceived(double, double);

    public slots:
        void on_dataReceived();

    private:
        bool consuming;
        CRioDataStream *crioDataStream;
        // end new datastream reader
        SensorConfig *sensorConfig;
        CoordinateHelper* coordinateHelper;
        FileHelper *fileHelper;
        DatabaseManager* dbManager;
        void handleGetCommand(int address);
        QString createLogText(DataObject dataObj);
        void writeInLogFile(Sensor* s, QString logTxt);
        const DataObject applyTransformation(QString dllName, DataObject val) const;
        const DataObject transformDataObject(DataObject iobj);
};

#endif // MESSAGECONSUMER_H
