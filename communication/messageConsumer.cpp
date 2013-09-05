#include "messageConsumer.h"
#include "server.h"
#include <QThread>

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

void MessageConsumer::on_dataReceived()
{
    checkQueue(true);
}

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
    QVector< QPair<QVariant, DataType::Types> > values;
    for (int i = 0; i < dataArrayLength; i++) {
        QPair<QVariant, DataType::Types> res = decodeDataValue();
        values.push_back(res);
        str += " " + res.first.toString();
    }
    qint64 ts = decodeTimestamp2(); // TODO
    DataObject* dataObj = new DataObject(address, values, ts);
    waitingData = 0;
    handleMessageData(dataObj);
    emit messageParsed(str+"\n");
    checkQueue(false); // force checking queue to parse data that would have arrived meanwhile
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
        //QByteArray invertedBytes = converter->invertBytes(valueBytes); // invert bytes because for double type Labview sends the bytes in reverse order
        double d = converter->byteArrayToDouble(valueBytes);
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
    printf("TS: %f, TS unix: %lld\n", ts, tsUnix);
    fflush(stdout);
    return tsUnix;
}

qint64 MessageConsumer::decodeTimestamp2()
{
    QByteArray tsBytes = readBytes(16);
    double seconds = converter->byteArrayToDouble(converter->getFirstBytesOfArray(tsBytes, 8));
    double fraction = converter->byteArrayToDouble(converter->getLastBytesOfArray(tsBytes, 8)); // always 0 in test
    //quint64 divider = 18446744073709551615; // 2^64 -1
    //double decimal = (double)fraction / divider;
    double ts = seconds + fraction;
    //qint64 tsUnix = TimeHelper::labviewTsToUnixTs(ts);
    qint64 tsUnix = round(ts * 1000); // milliseconds from epoch
    printf("TS: %f, TS unix: %lld\n", ts, tsUnix);
    fflush(stdout);
    return tsUnix;
}

/**
 * Handle data according to data type. Act specifically for each type of data (GPS, temperature, radiometer, etc)
 * @brief MessageConsumer::handleMessageData
 * @param address The address of the sensor
 * @param values The values of the log
 * @param ts The timestamp of the log
 */
void MessageConsumer::handleMessageData(DataObject* dataObj)
{
    if (sensorConfig->containsSensor(dataObj->getAddress()))
    {
        Sensor *s = sensorConfig->getSensor(dataObj->getAddress());
        // switch can not be used with QString
        switch (s->getType()->getId()) {
        case SensorList::GPS_position:
        {
            // GPS position
            double lat = dataObj->getValues()[0].first.toDouble();
            double lon = dataObj->getValues()[1].first.toDouble();
            double elevation = dataObj->getValues()[2].first.toDouble();
            // convert to CH1903 coordinates (east, north, h)
            QVector<double> swissCoordinates = coordinateHelper->WGS84toLV03(lat, lon, elevation);
            // get x,y position for map in UI
            QVector<double> mapPosition = coordinateHelper->LV03toUIMap(swissCoordinates[0], swissCoordinates[1]);
            if (mapPosition[0] != 0.0 && mapPosition[1] != 0.0)
            {
                // notify GUI only if coordinates are plausible
                emit gpsPointReceived(mapPosition[0], mapPosition[1]);
            }
            QString logFile = s->getFilename() + ".log";
            QString log = QString::number(s->getAddress()) + "\t" + QString::number(dataObj->getTimestamp(),'f',6) + "\t" + QString::number(lat,'f',6) + "\t" + QString::number(lon,'f',6) + "\t" + QString::number(elevation,'f',6);
            fileHelper->appendToFile(logFile, log);
            break;
        }
        case SensorList::PT100:
        {
            // dataObj->values contains only one temperature value
            double temp = dataObj->getValues()[0].first.toDouble();
            // save it to database
            dbManager->insertLogDoubleValue(dbManager->getTableName(Datastore::TemperatureLog), dataObj->getAddress(), dataObj->getTimestamp(), temp);
            break;
        }
        case SensorList::Wind_speed:
        case SensorList::Wind_direction:
        {
            // dataObj->values contains only one temperature value
            double value = dataObj->getValues()[0].first.toDouble();
            // save it to database
            dbManager->insertLogDoubleValue(dbManager->getTableName(Datastore::WindLog), dataObj->getAddress(), dataObj->getTimestamp(), value);
            break;
        }
        case SensorList::Radiometer:
        {
            // dataObj->values contains only one temperature value
            double value = dataObj->getValues()[0].first.toDouble();
            // save it to database
            dbManager->insertLogDoubleValue(dbManager->getTableName(Datastore::RadiometerLog), dataObj->getAddress(), dataObj->getTimestamp(), value);
            break;
        }
        default:
            // unknown sensor type
            printf("[MessageConsumer] Unknown sensor type !\n");
            fflush(stdout);
            break;
        }
    }
    else
    {
        printf("[MessageConsumer] Sensor not found in config !\n");
        fflush(stdout);
    }
}

/**
 * Parse a "CMD" message
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

void MessageConsumer::handleGetCommand(int address)
{
    switch (address) {
    case 5:
    {
        // send addresses array (config)
        quint8 command = MessageUtil::Set;
        quint8 engineAddr = 5;
        QByteArray data;
        data.push_back(command);
        data.push_back(converter->intToByteArray(2, 4));
        // add address
        data.push_back(converter->byteArrayForCmdParameterInt(engineAddr));
        // add stream array
        data.push_back(converter->byteArrayForCmdParameterStreamArray(sensorConfig->getSensors()));
        Server* s = (Server*)parent();
        s->sendCommandMessage(data);
        break;
    }
    default:
        printf("[MessageConsumer] handleGetCommand() - unhandled address !\n");
        fflush(stdout);
        break;
    }
}
