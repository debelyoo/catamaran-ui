#ifndef SENSORTYPE_H
#define SENSORTYPE_H

#include <QObject>

class SensorType
{
    public:
    explicit SensorType(int id, QString name = "", QString dbTableName = "", QString dllName = "");
    /// getters
    int getId();
    QString getName();
    QString getDbTableName();
    QString getDllName();

    /// setters
    void setId(int id);
    void setName(QString name);
    void setDbTableName(QString tn);
    void setDllName(QString dn);

    private:
        int id;
        QString name;
        QString dbTableName;
        QString dllName;
};

#endif // SENSORTYPE_H
