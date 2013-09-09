#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <QObject>
#include <QDateTime>
#include <QMap>
#include "sensorConfig.h"

/**
 * @brief The FileHelper class - Singleton
 */
class FileHelper
{

    public:
        static FileHelper* instance();

        void writeFile(QString filename, QString fileContent, bool isLog);
        void appendToFile(QString filename, QString text);
        void loadConfigFile(SensorConfig* sc);
        void createLogFiles(SensorConfig* sc);

    private:
        FileHelper() {
            logFolder = "logs";
        }
        FileHelper(const FileHelper &);
        FileHelper& operator=(const FileHelper &);

        static FileHelper* m_Instance;

        QString logFolder;
        QMap<QString, QString> logFiles;
        QString getLogFileName(QString prefix);
};

#endif // FILEHELPER_H
