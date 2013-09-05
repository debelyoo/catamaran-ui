#include "fileHelper.h"
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>

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
    QString logFolderPath = QDir::currentPath() + "/" + logFolder;
    QString filePath = logFolder + "/" +filename;
    QDir logDir(logFolderPath);
    if (!logDir.exists())
    {
        QDir::current().mkdir(logFolder);
    }

    QFile file(filePath);
    if (file.open(QIODevice::ReadWrite)) {
        QTextStream stream(&file);
        stream << fileContent << endl;
    }
}

void FileHelper::appendToFile(QString filename, QString text)
{
    QString filePath = QDir::currentPath() + "/"+ logFolder +"/" + filename;
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
    QString filePath = QDir::currentPath() + "/config2.txt";
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", file.errorString());
    }

    QTextStream in(&file);

    int count = 0;
    while(!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split("\t"); // separated by tab
        if (fields.length() == 7 && count > 0 && fields.at(0) != "")
        {
            int addr = QString(fields.at(0)).toInt();
            //int sType = QString(fields.at(2)).toInt();
            SensorType* sType = sensorConfig->getSensorTypes().value(QString(fields.at(2)).toInt());
            int displayInd = sensorConfig->getDisplayIndexForGraphName(fields.at(3));
            QString dateStr = QDateTime::currentDateTime().toString("ddMMyyyy_hhmmss");
            bool record = sensorConfig->qstringToBool(fields.at(4));
            QString currentLogFilename = "";
            Sensor *s = new Sensor(addr, fields.at(1), sType,
                                   displayInd, record,
                                   sensorConfig->qstringToBool(fields.at(5)), fields.at(6), currentLogFilename);
            sensorConfig->addSensor(s);
        }
        count++;
    }

    file.close();
}

/**
 * Create the log files for the sensor that have the 'record' flag set and log file prefix
 * @brief FileHelper::createLogFiles
 * @param sensorConfig The config object
 */
void FileHelper::createLogFiles(SensorConfig* sensorConfig)
{
    //qDebug() << "createLogFiles()";
    foreach(Sensor* s, sensorConfig->getSensors())
    {
        if (s->getRecord() && s->getLogFilePrefix() != "" && s->getCurrentLogFilename() == "")
        {
            QString dateStr = QDateTime::currentDateTime().toString("ddMMyyyy_hhmmss");
            QString currentLogFilename = s->getLogFilePrefix() + "_" + dateStr + ".log";
            //qDebug() << currentLogFilename;
            s->setCurrentLogFilename(currentLogFilename);
            writeFile(s->getCurrentLogFilename(), "");
        }
    }
}
