#include "fileHelper.h"
#include "database/databaseManager.h"
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>
#include "manager/sensortypemanager.h"

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
 * @param isLog Indicates if the file to write is a log file
 */
void FileHelper::writeFile(QString filename, QString fileContent, bool isLog)
{
    QString folderPath;
    if (isLog)
    {
        folderPath = QDir::currentPath() + "/" + logFolder + "/" + DatabaseManager::instance()->getCurrentMissionName().replace(':', ""); // colon can not be used in folder names
    }
    else
    {
        folderPath = QDir::currentPath();
    }
    QString filePath = folderPath + "/" +filename;
    QDir logDir(folderPath);
    if (!logDir.exists())
    {
        QDir::current().mkdir(folderPath);
    }

    QFile file(filePath);
    if (file.open(QIODevice::ReadWrite)) {
        if (fileContent != "")
        {
            QTextStream stream(&file);
            stream << fileContent << endl;
        }
    }
}

/**
 * Append text to log file
 * @brief FileHelper::appendToFile
 * @param filename
 * @param text
 */
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
 * @param sensorConfig The sensor configuration object
 */
bool FileHelper::loadConfigFile()
{
    SensorConfig* sensorConfig = SensorConfig::instance();
    bool res = false;
    QString configFilename = "config2.txt";
    QString filePath = QDir::currentPath() + "/"+ configFilename;
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", file.errorString() +" ["+configFilename+"]");
    }

    QTextStream in(&file);

    int count = 0;
    while(!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split("\t"); // separated by tab
        if (fields.length() == 7 && count > 0 && fields.at(0) != "")
        {
            QString addr = fields.at(0);
            //int sType = QString(fields.at(2)).toInt();
            const SensorType* sType = SensorTypeManager::instance()->type(fields.at(2));
            int displayInd = sensorConfig->getDisplayIndexForGraphName(fields.at(3));
            QString dateStr = QDateTime::currentDateTime().toString("ddMMyyyy_hhmmss");
            bool record = sensorConfig->qstringToBool(fields.at(4));
            QString currentLogFilename = "";
//            Sensor *s = new Sensor(addr, fields.at(1), sType,
//                                   displayInd, record,
//                                   sensorConfig->qstringToBool(fields.at(5)), fields.at(6), currentLogFilename);
            //sensorConfig->addSensor(s);
            res = true;
        }
        count++;
    }

    file.close();
    return res;
}

/**
 * Create the log files for the sensor that have the 'record' flag set and log file prefix
 * @brief FileHelper::createLogFiles
 * @param sensorConfig The config object
 */
void FileHelper::createLogFiles()
{
    SensorConfig* sensorConfig = SensorConfig::instance();
    qDebug() << "createLogFiles()";
    foreach(Sensor* s, sensorConfig->getSensors())
    {
        if (s->record() && s->logFilePrefix() != "" && s->currentLogFilename() == "")
        {
            QString currentLogFilename = getLogFileName(s->logFilePrefix());
            qDebug() << currentLogFilename;
            s->setCurrentLogFilename(currentLogFilename);
            writeFile(s->currentLogFilename(), "", true);
        }
    }
}

bool FileHelper::loadSensorTypesFile()
{
    bool res = false;
    QString sensorTypesFile = "sensortypes.txt";
    QString filePath = QDir::currentPath() + "/"+ sensorTypesFile;
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", file.errorString() + " ["+sensorTypesFile+"]");
    }

    QTextStream in(&file);

    int count = 0;
    while(!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split("\t"); // separated by tab
        if (count > 0 && fields.at(0) != "")
        {
            //int id = QString(fields.at(0)).toInt();
            //SensorType* st = new SensorType(fields.at(1), fields.at(2), fields.at(3).toInt());
            //sensorConfig->addSensorType(st);
            SensorTypeManager::instance()->createType(fields.at(1));
            res = true;
        }
        count++;
    }

    file.close();
    return res;
}

/**
 * Get the name of the log file. Returns existing filename if file with same prefix already exists
 * @brief FileHelper::getLogFileName
 * @param prefix The prefix of the log file
 * @return The name of the logfile
 */
QString FileHelper::getLogFileName(QString prefix)
{
    QString logFileName;
    if(logFiles.contains(prefix))
    {
        logFileName = logFiles.value(prefix);
    }
    else
    {
        QString dateStr = QDateTime::currentDateTime().toString("ddMMyyyy_hhmmss");
        logFileName = prefix + "_" + dateStr + ".log";
        logFiles.insert(prefix, logFileName);
    }
    return logFileName;
}
