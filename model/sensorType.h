#ifndef SENSORTYPE_H
#define SENSORTYPE_H

#include <QObject>

class SensorType
{
    public:
    explicit SensorType(int id, QString name = "", QString dbTableName = "");
    /// getters
    int getId();
    QString getName();
    QString getDbTableName();

    /// setters
    void setId(int id);
    void setName(QString name);
    void setDbTableName(QString tn);

    private:
        int id;
        QString name;
        QString dbTableName;
};

#endif // SENSORTYPE_H
