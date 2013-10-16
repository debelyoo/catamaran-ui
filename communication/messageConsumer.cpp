#include "messageConsumer.h"
#include "server.h"
#include <QThread>
#include <QLibrary>
#include <QDir>

MessageConsumer::MessageConsumer(QObject *parent, CRioDataStream *ds):
    //crioDataStream(&crioByteArrayIn, QIODevice::ReadWrite)
    QObject(parent),
    crioDataStream(ds)
{
  sensorConfig = SensorConfig::instance();
  fileHelper = FileHelper::instance();
  dbManager = DatabaseManager::instance();
  consuming = false;
}

/**
 * @brief MessageConsumer::on_dataReceived
 */
void MessageConsumer::on_dataReceived()
{
    //checkQueue(true);
    if(!consuming){
        consuming = true;
        while(crioDataStream->device()->bytesAvailable() >= 5){
            //qDebug() << "Try to decode Message (buffer length:"<<crioDataStream->device()->bytesAvailable()<<")";
            // New datastream reader
            //qDebug() << "ba : " << crioByteArrayIn.toHex();
            CRioMessage crioMessage(*crioDataStream);
            //qDebug() << "New CRioMessage : type = " << crioMessage.type();
            if(crioMessage.isValid()){

                switch(crioMessage.type()){
                case CRIO::CMD:
                {
                    CRioCommand *p = static_cast<CRioCommand *>(crioMessage.content());
                    //qDebug() << "\tCommand: cmd = " << p->command() << ", addr = " << p->address();
                    if(p->command() == CRIO::CMD_GET){
                        handleGetCommand((int)p->address());
                    }
                }
                    break;
                case CRIO::DATA:
                {
                    CRioData *p = static_cast<CRioData *>(crioMessage.content());
                    /*
                    QVector<DataValue> values;
                    for (int i = 0; i < p->data.count(); i++) {
                        CRIO::PolymorphicData &d = p->data[i];
                        DataValue val(d.value, d.cRIOType());
                        values.push_back(val);
                    }
                    DataObject dataObj = DataObject((quint8)p->address.toUShort(), values, p->timestamp.unixTimestamp);
                    */
                    handleMessageData(*p);
                }
                    break;
                default:
                    break;
                }
            }
        }
        consuming = false;
    }

    // end new datastream reader
}

/**
 * @brief MessageConsumer::transformDataObject
 * @param iobj object to transform
 * @return transformed object
 */
CRioData MessageConsumer::transformDataObject(CRioData &iobj){
    Sensor *s = sensorConfig->getSensor(iobj.address);
    return applyTransformation(s->type()->getDllName(), iobj);
}

/**
 * Handle data according to data type. Act specifically for each type of data (GPS, temperature, radiometer, etc)
 * @brief MessageConsumer::handleMessageData
 * @param address The address of the sensor
 * @param values The values of the log
 * @param ts The timestamp of the log
 */
void MessageConsumer::handleMessageData(CRioData &idataObj)
{
    bool valid;
    quint8 intAddress = idataObj.address.toInt(&valid);
    if (valid & sensorConfig->containsSensor(idataObj.address))
    {
        // Temporary modification for test on 09.10.2013
        if(idataObj.address == "53"){
            QVariantList tmp; tmp.append(idataObj.data()[1]);
            idataObj = CRioData(intAddress, tmp, idataObj.timestamp);
        }
        // End of temporary modification for 09.10.2013

        CRioData dataObj = transformDataObject(idataObj);
//        qDebug() << "New data: addr=" << dataObj.getAddress() << " TS=" << QDateTime::fromMSecsSinceEpoch(dataObj.getTimestamp());
//        foreach(DataValue dv, dataObj.getValues()){
//            qDebug() << "\t" << dv.first;
//        }

        Sensor *s = sensorConfig->getSensor(idataObj.address);
        // switch can not be used with QString
        switch (s->type()->getId()) {
        case SensorList::GPS_position:
        {
            // GPS position
            double lat = dataObj.data()[0].toDouble();
            double lon = dataObj.data()[1].toDouble();
            double elevation = dataObj.data()[2].toDouble();
            // convert to CH1903 coordinates (east, north, h)
            QVector<double> swissCoordinates = coordinateHelper->WGS84toLV03(lat, lon, elevation);
            // get x,y position for map in UI
            QPointF mapPosition = coordinateHelper->LV03toUIMap(swissCoordinates[0], swissCoordinates[1]);
            if (mapPosition.x() != 0.0 && mapPosition.y() != 0.0)
            {
                // notify GUI only if coordinates are plausible
                emit gpsPointReceived(mapPosition.x(), mapPosition.y());
            }
            //QString log = QString::number(s->getAddress()) + "\t" + QString::number(dataObj->getTimestamp(),'f',6) + "\t" + QString::number(lat,'f',6) + "\t" + QString::number(lon,'f',6) + "\t" + QString::number(elevation,'f',6);
            QString log = createLogText(dataObj);
            writeInLogFile(s, log);
            break;
        }
        case SensorList::PT100:
        {
            // dataObj->values contains only one temperature value
            double temp = dataObj.data()[0].toDouble();
            // save it to database
            dbManager->insertLogDoubleValue(s->type()->getDbTableName(), intAddress, dataObj.timestamp.unixTimestamp, temp);
            // log it in log file
            QString log = createLogText(dataObj);
            writeInLogFile(s, log);
            break;
        }
        case SensorList::Wind_speed:
        case SensorList::Wind_direction:
        {
            // dataObj->values contains only one value
            double value = dataObj.data()[0].toDouble();
            // save it to database
            dbManager->insertLogDoubleValue(s->type()->getDbTableName(), intAddress, dataObj.timestamp.unixTimestamp, value);
            // log it in log file
            QString log = createLogText(dataObj);
            writeInLogFile(s, log);
            break;
        }
        case SensorList::Radiometer:
        {
            // dataObj->values contains only one value
            double value = dataObj.data()[0].toDouble();
            // save it to database
            dbManager->insertLogDoubleValue(s->type()->getDbTableName(), intAddress, dataObj.timestamp.unixTimestamp, value);
            // log it in log file
            QString log = createLogText(dataObj);
            writeInLogFile(s, log);
            break;
        }
        default:
            // unknown sensor type
            qDebug() << "[MessageConsumer] Unknown sensor type !\n";
            break;
        }
        QString outputStr = QDateTime::fromMSecsSinceEpoch(dataObj.timestamp.unixTimestamp).toString("hh:mm:ss.zzz") + ": Addr(%1)=";
        outputStr = outputStr.arg(dataObj.address);
        foreach(QVariant dv, dataObj.data()){
            outputStr += QString("[%1]").arg(dv.toDouble());
        }
        outputStr += "\r\n";
        emit messageParsed(outputStr);
    }
    else
    {
        qDebug() << "[MessageConsumer] Sensor not found in config !";
    }
}

MessageConsumer::~MessageConsumer()
{

}


/**
 * Handle a received GET command
 * @brief MessageConsumer::handleGetCommand
 * @param address The address specified in the GET command
 */
void MessageConsumer::handleGetCommand(int address)
{
    switch (address) {
    case 5:
    {
        Server* s = (Server*)parent();
        s->sendMessage(CRIO::setSensorsConfig(sensorConfig->getSensors()));
        break;
    }
    default:
        printf("[MessageConsumer] handleGetCommand() - unhandled address !\n");
        fflush(stdout);
        break;
    }
}

/**
 * Create a text line to go in logfile
 * @brief MessageConsumer::createLogText
 * @param dataObj The log details
 * @return The log as text line (String)
 */
QString MessageConsumer::createLogText(const CRioData &dataObj)
{
    QString log;
    QString valuesAsText;
    foreach (CRIO::PolymorphicData val, dataObj.data()) {
        QMetaType::Type type = (QMetaType::Type)val.value.type();
        switch (type) {
        case QMetaType::Double:
            valuesAsText += "\t"+ QString::number(val.value.toDouble(), 'f', 6);
            break;
        default:
            break;
        }
    }
    log = dataObj.address + "\t" + QDateTime::fromMSecsSinceEpoch(dataObj.timestamp.unixTimestamp).toString("hh:mm:ss:zzz: ") + valuesAsText;
    return log;
}

/**
 * Write some text in log file
 * @brief MessageConsumer::writeInLogFile
 * @param s
 * @param logTxt
 */
void MessageConsumer::writeInLogFile(Sensor* s, QString logTxt)
{
    QString logFile = s->currentLogFilename();
    if (logFile != "")
    {
        fileHelper->appendToFile(logFile, logTxt);
    }
}

/**
 * Apply a transformation to the value. Transformation is defined in external library (DLL)
 * @brief MessageConsumer::applyTransformation
 * @param dllName The name of the external library
 * @param iobj The object to transform
 * @return The transformed object
 */
CRioData MessageConsumer::applyTransformation(QString dllName, CRioData &iobj) const
{
    if (dllName != "")
    {
        TransformationBaseClass *tPtr = TransformationManager::instance()->getTransformation(dllName);
        if(tPtr){
            return tPtr->applyTransform(iobj, (IDataMessageReceiver*)this, NULL, NULL);
        }
    }
    return iobj;
}
