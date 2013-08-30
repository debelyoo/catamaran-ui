#include "messageConsumer.h";
#include "server.h"
#include <QThread>

MessageConsumer::MessageConsumer(QObject *parent, QQueue<char> *q) :
QObject(parent)
{
    sensorConfig = SensorConfig::instance();
    fileHelper = FileHelper::instance();
    converter = ByteArrayConverter::instance();
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
        uint msgType = converter->byteArrayToUInt8(readBytes(1));
        uint msgLength = converter->byteArrayToUInt32(readBytes(4));
        switch (msgType) {
        case 0:
        {
            int qs = queue->size();
            if (qs < msgLength) {
                waitingData = msgLength;
                //printf("[MessageConsumer] Wait data !\n");
                //fflush(stdout);
                return; // need to wait for more data
            }
            //str = "read >> DATA of length "+ QString::number(msgLength)+" \n";
            parseDataMessage();
            break;
        }
        case 1:
            str = "read >> CMD message\n";
            emit messageParsed(str);
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
    QVector< QPair<QString, DataType::Types> > values;
    for (int i = 0; i < dataArrayLength; i++) {
        QPair<QString, DataType::Types> res = decodeDataValue();
        values.push_back(res);
        str += " " + res.first;
    }
    double ts = decodeTimestamp();
    waitingData = 0;
    handleMessageData(address, values, ts);
    emit messageParsed(str+"\n");
    checkQueue(false); // force checking queue to parse data that would have arrived meanwhile
    //return str;
}

/**
 * Decode the value from Byte array according to its type.
 * @brief MessageConsumer::decodeDataValue
 * @return
 */
QPair<QString, DataType::Types> MessageConsumer::decodeDataValue()
{
    QPair<QString, DataType::Types> pair;
    bool ok;
    int valueLength = converter->byteArrayToUInt32(readBytes(4));
    QByteArray valueBytes = readBytes(valueLength);
    int valueType = converter->byteArrayToUInt8(readBytes(1));
    QString valStr;
    uint v;
    double d;
    switch (valueType) {
    case DataType::Double: // double
    {
        QByteArray invertedBytes = converter->invertBytes(valueBytes); // invert bytes because for double type Labview sends the bytes in reverse order
        d = converter->byteArrayToDouble(invertedBytes);
        //msg = "["+ QString::number(d) +", Double]";
        valStr = QString::number(d, 'f', 6);
        break;
    }
    case DataType::UInt32: // uint32
        v = converter->byteArrayToUInt32(valueBytes);
        //msg = "["+ QString::number(v) +", uint32]";
        valStr = QString::number(v);
        break;
    case DataType::UInt16:
        v = converter->byteArrayToUInt16(valueBytes);
        //msg = "["+ QString::number(v) +", uint16]";
        valStr = QString::number(v);
        break;
    default:
        valStr = "NaN";
        break;
    }
    pair.first = valStr;
    pair.second = (DataType::Types)valueType;
    return pair;
}

/**
 * Decode the timestamp from Byte array.
 * @brief MessageConsumer::decodeTimestamp
 * @return
 */
double MessageConsumer::decodeTimestamp()
{
    bool ok;
    QByteArray tsBytes = readBytes(16);
    qint64 seconds = converter->byteArrayToInt64(converter->getFirstBytesOfArray(tsBytes, 8));
    //qint64 secondsBis = converter->byteArrayToInt64Bis(converter->getFirstBytesOfArray(tsBytes, 8));
    quint64 fraction = converter->byteArrayToUInt64(converter->getLastBytesOfArray(tsBytes, 8));
    quint64 divider = 18446744073709551615; // 2^64 -1
    double decimal = (double)fraction / divider;
    double ts = seconds + decimal;
    //QString tsStr = "some TS";
    printf("TS: %f", ts);
    fflush(stdout);
    return ts;
}

/**
 * Handle data according to data type. Act specifically for each type of data (GPS, temperature, radiometer, etc)
 * @brief MessageConsumer::handleMessageData
 * @param address The address of the sensor
 * @param values The values of the log
 * @param ts The timestamp of the log
 */
void MessageConsumer::handleMessageData(int address, QVector< QPair<QString, DataType::Types> > values, double ts)
{
    if (sensorConfig->containsSensor(address))
    {
        Sensor *s = sensorConfig->getSensor(address);
        // switch can not be used with QString

        switch (s->getType()) {
        case SensorType::GPS_position:
        {
            // GPS position
            double lat = values[0].first.toDouble();
            double lon = values[1].first.toDouble();
            double elevation = values[2].first.toDouble();
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
            QString log = QString::number(s->getAddress()) + "\t" + QString::number(ts,'f',6) + "\t" + QString::number(lat,'f',6) + "\t" + QString::number(lon,'f',6) + "\t" + QString::number(elevation,'f',6);
            fileHelper->appendToFile(logFile, log);
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
        int address = converter->byteArrayToUInt32(readBytes(paramLength));
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
        // send addresses array (config)
        break;
    default:
        printf("[MessageConsumer] handleGetCommand() - unhandled address !\n");
        fflush(stdout);
        break;
    }
}
