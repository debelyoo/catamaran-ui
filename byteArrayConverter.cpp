#include "byteArrayConverter.h";

ByteArrayConverter* ByteArrayConverter::m_Instance = 0;

ByteArrayConverter* ByteArrayConverter::instance()
{
    if (!m_Instance)   // Only allow one instance of class to be generated.
    {
        m_Instance = new ByteArrayConverter;
    }
    return m_Instance;
}

quint8 ByteArrayConverter::byteArrayToUInt8(QByteArray ba) {
    return ba[0];
}
quint16 ByteArrayConverter::byteArrayToUInt16(QByteArray ba) {
    QDataStream ds(ba);
    quint16 v;
    ds >> v;
    return v;
}
quint32 ByteArrayConverter::byteArrayToUInt32(QByteArray ba) {
    QDataStream ds(ba);
    quint32 v;
    ds >> v;
    return v;
}
quint64 ByteArrayConverter::byteArrayToUInt64(QByteArray ba) {
    QDataStream ds(ba);
    quint64 v;
    ds >> v;
    return v;
}
qint64 ByteArrayConverter::byteArrayToInt64(QByteArray ba) {
    QDataStream ds(ba);
    qint64 v;
    ds >> v;
    return v;
}
/*qint64 ByteArrayConverter::byteArrayToInt64Bis(QByteArray ba) {
    qint64 v;
    v = ((qint64*)ba.constData())[0];
    return v;
}*/
double ByteArrayConverter::byteArrayToDouble(QByteArray ba) {
    double d;
    d = ((double*)ba.constData())[0];
    return d;
}

QByteArray ByteArrayConverter::intToByteArray(int val, int byteArraySize) {
    QByteArray byteArray;
    QVector<unsigned char> ba(byteArraySize);
    //char ba[byteArraySize];
    for (int i = 0; i < byteArraySize; i++) {
        ba[byteArraySize - 1 - i] = (val >> (i * 8));
    }
    for (int i = 0; i < ba.size(); i++) {
        byteArray.push_back(ba[i]);
    }
    return byteArray;
}

QByteArray ByteArrayConverter::getFirstBytesOfArray(QByteArray ba, int n)
{
    QByteArray partBa;
    for (int i = 0; i < n; i++)
    {
        partBa.push_back(ba[i]);
    }
    return partBa;
}

QByteArray ByteArrayConverter::getLastBytesOfArray(QByteArray ba, int n)
{
    QByteArray partBa;
    int offset = ba.size() - n;
    for (int i = offset; i < offset+n; i++)
    {
        partBa.push_back(ba[i]);
    }
    return partBa;
}

QByteArray ByteArrayConverter::invertBytes(QByteArray ba)
{
    QByteArray invertedBa;
    for (int i = 0; i < ba.length(); i++)
    {
        invertedBa[i] = ba[ba.length() - 1 -i];
    }
    return invertedBa;
}

/**
 * Create the byte array for a value of a CMD message parameter
 * @brief ByteArrayConverter::byteArrayForCmdParameter
 * @param val The value to encode
 * @return A QByteArray with length and value
 */
QByteArray ByteArrayConverter::byteArrayForCmdParameterInt(int val)
{
    QByteArray ba;
    QByteArray length = intToByteArray(1, 4);
    ba.push_back(length);
    ba.push_back(val);
    return ba;
}

/**
 * Create the byte array for the array of values to send to the cRIO (addresses to stream)
 * @brief ByteArrayConverter::byteArrayForCmdParameterStreamArray
 * @param sensors The sensors in the config
 * @return The Byte array
 */
QByteArray ByteArrayConverter::byteArrayForCmdParameterStreamArray(QList<Sensor*> sensors)
{
    QByteArray ba;
    foreach (Sensor* s, sensors) {
        if (s->getStream())
        {
            ba.push_back(s->getStream());
            ba.push_back(s->getAddress());
        }
    }
    // add array length (uint32) before array
    ba.prepend(intToByteArray(ba.length(), 4));
    // add string length (uint32) before (array len, array)
    ba.prepend(intToByteArray(ba.length(), 4));
    return ba;
}
