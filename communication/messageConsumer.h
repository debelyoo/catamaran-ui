#ifndef MESSAGECONSUMER_H
#define MESSAGECONSUMER_H

#include "sensorConfig.h"
#include "util/coordinateHelper.h"
#include "util/fileHelper.h"
#include "util/timeHelper.h"
#include "util/database/databaseManager.h"
#include "communication/criodata.h"
#include "communication/criocommand.h"
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
        void handleDataMessage(CRioData &dataObj);
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
        QString createLogText(const CRioData &dataObj);
        void writeInLogFile(Sensor* s, QString logTxt);
        void writeInLogFile(Sensor *s, const QByteArray &logTxt);
        CRioData applyTransformation(QString dllName, CRioData &val) const;
        CRioData transformDataObject(CRioData &iobj);
};

#endif // MESSAGECONSUMER_H
