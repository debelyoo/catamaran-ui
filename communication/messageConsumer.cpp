#include "messageConsumer.h"
#include "server.h"
#include <QThread>
#include <QLibrary>
#include <QDir>

/**
 * @brief MessageConsumer::MessageConsumer
 * @param parent
 * @param q
 */
MessageConsumer::MessageConsumer(QObject *parent, QQueue<char> *q) :
QObject(parent)
{
    sensorConfig = SensorConfig::instance();
    fileHelper = FileHelper::instance();
    converter = ByteArrayConverter::instance();
    dbManager = DatabaseManager::instance();
    queue = q;
    consuming = false;
    waitingData = 0;
}

/**
 * @brief MessageConsumer::on_dataReceived
 */
void MessageConsumer::on_dataReceived()
{
    checkQueue(true);
}

/**
 * @brief MessageConsumer::checkQueue
 * @param checkIfConsuming
 */
void MessageConsumer::checkQueue(bool checkIfConsuming)
{
    /*QString str = "[MessageConsumer] checkQueue() - check: "+ QString::number(checkIfConsuming) +", consuming: "+ QString::number(consuming) +", waitingData: "+ QString::number(waitingData) +" \n";
    printf(str.toLatin1().data());
    fflush(stdout);
    */

    if (checkIfConsuming && !consuming) {
        consuming = true;
        if (waitingData > 0) {
            // we are waiting for missing data so parse, so check if we have enough data and act accordingly
            if (queue->size() >= waitingData) {
                parseDataMessage();
            }
            consuming = false;
        } else {
            readQueue();
            consuming = false;
        }
    }
}

/**
 * @brief MessageConsumer::readQueue
 */
void MessageConsumer::readQueue()
{
    if (!queue->isEmpty()) {
        QString str;
        int msgType = converter->byteArrayToUInt8(readBytes(1));
        int msgLength = converter->byteArrayToUInt32(readBytes(4));
        int qs = queue->size();
        if (qs < msgLength) {
            waitingData = msgLength;
            //printf("[MessageConsumer] Wait data !\n");
            //fflush(stdout);
            return; // need to wait for more data
        }
        //str = "read >> DATA of length "+ QString::number(msgLength)+" \n";
        switch (msgType) {
        case 0:
            parseDataMessage();
            break;
        case 1:
            parseCmdMessage();
            break;
        default:
            str = "read >> unknown message\n";
            emit messageParsed(str);
            break;
        }
    } else {
        //str = "queue is empty !\n";
        printf("[MessageConsumer] queue is empty !\n");
        fflush(stdout);
    }
}

/**
 * @brief MessageConsumer::readBytes
 * @param nbBytesToRead
 * @return
 */
QByteArray MessageConsumer::readBytes(int nbBytesToRead)
{
    QByteArray ba;
    int i = 0;
    while(!queue->isEmpty() && nbBytesToRead > 0) {
        //QString newChar = QString().sprintf("%c", queue->dequeue());
        char c = queue->dequeue();
        ba.append(c);
        //ba[i] = queue->dequeue();
        nbBytesToRead--;
        i++;
    }

    return ba;
}

/**
 * Parse a "DATA" message
 * @brief MessageConsumer::parseDataMessage
 */
void MessageConsumer::parseDataMessage()
{
    //printf("[MessageConsumer] parseDataMessage !\n");
    //fflush(stdout);
    QString str;
    str = "[DATA] ";
    int address = converter->byteArrayToUInt8(readBytes(1));
    str += "address: "+ QString::number(address);
    int dataArrayLength = converter->byteArrayToUInt32(readBytes(4));
    QVector<DataValue> values;
    for (int i = 0; i < dataArrayLength; i++) {
        DataValue val = decodeDataValue();
        values.push_back(val);
        str += " " + val.first.toString();
    }
    qint64 ts = decodeTimestamp(); // TODO
    DataObject dataObj = DataObject(address, values, ts);
    waitingData = 0;
    handleMessageData(dataObj);
    emit messageParsed(str+"\n");
    checkQueue(false); // force checking queue to parse data that would have arrived meanwhile
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
 * Decode the value from Byte array according to its type.
 * @brief MessageConsumer::decodeDataValue
 * @return
 */
QPair<QVariant, DataType::Types> MessageConsumer::decodeDataValue()
{
    QPair<QVariant, DataType::Types> pair;
    int valueLength = converter->byteArrayToUInt32(readBytes(4));
    QByteArray valueBytes = readBytes(valueLength);
    int valueType = converter->byteArrayToUInt8(readBytes(1));
    //QString valStr;
    QVariant valVar;
    switch (valueType) {
    case DataType::Double: // double
    {
        QByteArray invertedBytes = converter->invertBytes(valueBytes); // invert bytes because for double type Labview sends the bytes in reverse order
        double d = converter->byteArrayToDouble(invertedBytes);
        valVar = QVariant(d);
        break;
    }
    case DataType::UInt32: // uint32
    {
        quint32 v = converter->byteArrayToUInt32(valueBytes);
        valVar = QVariant(v);
        break;
    }
    case DataType::UInt16:
    {
        quint16 v = converter->byteArrayToUInt16(valueBytes);
        valVar = QVariant(v);
        break;
    }
    case DataType::UInt8:
    {
        quint8 v = converter->byteArrayToUInt8(valueBytes);
        valVar = QVariant(v);
        break;
    }
    case DataType::Int32:
    {
        qint32 v = converter->byteArrayToInt32(valueBytes);
        valVar = QVariant(v);
        break;
    }
    case DataType::Int16:
    {
        qint16 v = converter->byteArrayToInt16(valueBytes);
        valVar = QVariant(v);
        break;
    }
    case DataType::Int8:
    {
        qint8 v = converter->byteArrayToInt8(valueBytes);
        valVar = QVariant(v);
        break;
    }
    default:
        //valStr = "NaN";
        valVar = QVariant();
        break;
    }
    pair.first = valVar;
    pair.second = (DataType::Types)valueType;
    return pair;
}

/**
 * Decode the timestamp from Byte array.
 * @brief MessageConsumer::decodeTimestamp
 * @return
 */
qint64 MessageConsumer::decodeTimestamp()
{
    QByteArray tsBytes = readBytes(16);
    qint64 seconds = converter->byteArrayToInt64(converter->getFirstBytesOfArray(tsBytes, 8));
    quint64 fraction = converter->byteArrayToUInt64(converter->getLastBytesOfArray(tsBytes, 8));
    quint64 divider = 18446744073709551615; // 2^64 -1
    double decimal = (double)fraction / divider;
    double ts = seconds + decimal; // labview timestamp: seconds since the epoch 01/01/1904 00:00:00.00 UTC
    qint64 tsUnix = TimeHelper::labviewTsToUnixTs(ts);
    char str[128];
    sprintf(str, "TS: %f, TS unix: %lld\n", ts, tsUnix);
    qDebug() << str;
    return tsUnix;
}

/// For test only
qint64 MessageConsumer::decodeTimestamp2()
{
    QByteArray tsBytes = readBytes(16);
    double seconds = converter->byteArrayToDouble(converter->getFirstBytesOfArray(tsBytes, 8));
    double fraction = converter->byteArrayToDouble(converter->getLastBytesOfArray(tsBytes, 8)); // always 0 in test
    double ts = seconds + fraction;
    qint64 tsUnix = round(ts * 1000); // milliseconds from epoch
    char str[128];
    sprintf(str, "TS: %f, TS unix: %lld", ts, tsUnix);
    qDebug() << str;
    return tsUnix;
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

/**
 * Parse a "CMD" message, detects command type and call the corresponding action
 * @brief MessageConsumer::parseCmdMessage
 */
void MessageConsumer::parseCmdMessage()
{
    QString str;
    str = "[CMD] ";
    int cmd = converter->byteArrayToUInt8(readBytes(1));
    str += QString::number(cmd);
    int paramArrayLength = converter->byteArrayToUInt32(readBytes(4));
    switch (cmd) {
    case MessageUtil::Get:
    {
        // only one param (address)
        int paramLength = converter->byteArrayToUInt32(readBytes(4));
        int address = converter->byteArrayToUInt8(readBytes(paramLength));
        str += " " + QString::number(address);
        handleGetCommand(address);
        break;
    }
    case MessageUtil::Set:
    {
        // only when cRIO sends reference timestamp, params: address, timestamp
        int param1Length = converter->byteArrayToUInt32(readBytes(4));
        int param1 = converter->byteArrayToUInt32(readBytes(param1Length));
        int param2Length = converter->byteArrayToUInt32(readBytes(4));
        int param2 = converter->byteArrayToDouble(readBytes(param2Length));
        str += " " + QString::number(param1) + " " + QString::number(param2, 'f', 8);
        break;
    }
    default:
        break;
    }
    waitingData = 0;
    emit messageParsed(str+"\n");
    checkQueue(false); // force checking queue to parse data that would have arrived meanwhile
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
        // send addresses array (config)        
        Server* s = (Server*)parent();
        QByteArray data = s->prepareConfigMessage();
        s->sendCommandMessage(data);
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
        /*
        QString libFolderPath = QDir::currentPath() + "/lib";
        QLibrary library(libFolderPath + "/"+ dllName);
        bool okLoad = library.load(); // check load DLL file successful or not

        typedef DataObject (*TransformFunction)(DataObject, IDataMessageReceiver*);
        */
        TransformationBaseClass *tPtr = TransformationManager::instance()->getTransformation(dllName);
        if(tPtr){
            return tPtr->applyTransform(iobj, (IDataMessageReceiver*)this);
        }

        /*
        if (okLoad)
        {
            TransformFunction trsf = (TransformFunction) library.resolve("applyTransform");
            if (trsf)
                return trsf(iobj, (IDataMessageReceiver*)this);
        }
        */
    }
    return iobj;
}
