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
                    //qDebug() << "\tData: address = " << p->address << " ts = " << p->timestamp.timestamp << " datas :";
                    QVector<DataValue> values;

                    for (int i = 0; i < p->data.count(); i++) {
                        CRIO::PolymorphicData &d = p->data[i];
                        DataValue val(d.value, d.cRIOType());
                        values.push_back(val);
                        //str += " " + val.first.toString();
                        //qDebug() << "\t\t"<<(i+1)<<") " << val;
                    }
                    DataObject dataObj = DataObject((quint8)p->address.toUShort(), values, p->timestamp.unixTimestamp);
                    handleMessageData(dataObj);
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
const DataObject MessageConsumer::transformDataObject(DataObject iobj){
    Sensor *s = sensorConfig->getSensor(iobj.getAddress());
    return applyTransformation(s->getType()->getDllName(), iobj);
}

/**
 * Handle data according to data type. Act specifically for each type of data (GPS, temperature, radiometer, etc)
 * @brief MessageConsumer::handleMessageData
 * @param address The address of the sensor
 * @param values The values of the log
 * @param ts The timestamp of the log
 */
void MessageConsumer::handleMessageData(DataObject idataObj)
{
    if (sensorConfig->containsSensor(idataObj.getAddress()))
    {
        DataObject dataObj = transformDataObject(idataObj);

        Sensor *s = sensorConfig->getSensor(dataObj.getAddress());
        // switch can not be used with QString
        switch (s->getType()->getId()) {
        case SensorList::GPS_position:
        {
            // GPS position
            double lat = dataObj.getValues()[0].first.toDouble();
            double lon = dataObj.getValues()[1].first.toDouble();
            double elevation = dataObj.getValues()[2].first.toDouble();
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
            double temp = dataObj.getValues()[0].first.toDouble();
            // save it to database
            dbManager->insertLogDoubleValue(s->getType()->getDbTableName(), dataObj.getAddress(), dataObj.getTimestamp(), temp);
            // log it in log file
            QString log = createLogText(dataObj);
            writeInLogFile(s, log);
            break;
        }
        case SensorList::Wind_speed:
        case SensorList::Wind_direction:
        {
            // dataObj->values contains only one value
            double value = dataObj.getValues()[0].first.toDouble();
            // save it to database
            dbManager->insertLogDoubleValue(s->getType()->getDbTableName(), dataObj.getAddress(), dataObj.getTimestamp(), value);
            // log it in log file
            QString log = createLogText(dataObj);
            writeInLogFile(s, log);
            break;
        }
        case SensorList::Radiometer:
        {
            // dataObj->values contains only one value
            double value = dataObj.getValues()[0].first.toDouble();
            // save it to database
            dbManager->insertLogDoubleValue(s->getType()->getDbTableName(), dataObj.getAddress(), dataObj.getTimestamp(), value);
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
QString MessageConsumer::createLogText(DataObject dataObj)
{
    QString log;
    QString valuesAsText;
    QPair<QVariant, DataType::Types> val;
    foreach (val, dataObj.getValues()) {
        QMetaType::Type type = (QMetaType::Type)val.first.type();
        switch (type) {
        case QMetaType::Double:
            valuesAsText += "\t"+ QString::number(val.first.toDouble(), 'f', 6);
            break;
        default:
            break;
        }
    }
    log = QString::number(dataObj.getAddress()) + "\t" + QString::number(dataObj.getTimestamp()) + valuesAsText;
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
    QString logFile = s->getCurrentLogFilename();
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
const DataObject MessageConsumer::applyTransformation(QString dllName, DataObject iobj) const
{
    if (dllName != "")
    {
        TransformationBaseClass *tPtr = TransformationManager::instance()->getTransformation(dllName);
        if(tPtr){
            return tPtr->applyTransform(iobj, (IDataMessageReceiver*)this);
        }
    }
    return iobj;
}
