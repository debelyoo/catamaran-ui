#include "fileHelper.h"
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>

FileHelper* FileHelper::m_Instance = 0;

FileHelper* FileHelper::instance()
{
    if (!m_Instance)   // Only allow one instance of class to be generated.
    {
        m_Instance = new FileHelper;
    }
    return m_Instance;
}

/**
 * Write the 'fileContent' in a file in the current directory
 * @brief FileHelper::writeFile
 * @param filename The name of the file to write
 * @param fileContent The content of the file
 */
void FileHelper::writeFile(QString filename, QString fileContent)
{
    QString filePath = QDir::currentPath() + "/" + filename;
    QFile file(filePath);
    if (file.open(QIODevice::ReadWrite)) {
        QTextStream stream(&file);
        stream << fileContent << endl;
    }
}

void FileHelper::appendToFile(QString filename, QString text)
{
    QString filePath = QDir::currentPath() + "/" + filename;
    QFile file(filePath);
    if (file.open(QIODevice::ReadWrite)) {
        file.seek(file.size());
        QTextStream stream(&file);
        stream << text << endl;
    }
}

/**
 * Load the config file in memory (populate sensors list in SensorConfig singleton)
 * @brief MainWindow::loadConfigFile
 */
void FileHelper::loadConfigFile(SensorConfig* sensorConfig)
{
    QString filePath = QDir::currentPath() + "/config.txt";
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", file.errorString());
    }

    QTextStream in(&file);

    int count = 0;
    while(!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split("\t"); // separate by tab
        if (fields.length() == 7 && count > 0 && fields.at(0) != "")
        {
            int addr = QString(fields.at(0)).toInt();
            int sType = QString(fields.at(2)).toInt();
            Sensor *s = new Sensor(addr, fields.at(1), sType,
                                   fields.at(3), sensorConfig->qstringToBool(fields.at(4)),
                                   sensorConfig->qstringToBool(fields.at(5)), fields.at(6));
            sensorConfig->addSensor(s);
        }
        count++;
    }

    file.close();
}

void FileHelper::createLogFiles(SensorConfig* sensorConfig)
{
    foreach(Sensor* s, sensorConfig->getSensors())
    {
        if (s->getRecord() && s->getFilename() != "")
        {
            QString filename = s->getFilename() + ".log";
            writeFile(filename, "");
        }
    }
}
