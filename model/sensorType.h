#ifndef SENSORTYPE_H
#define SENSORTYPE_H

#include <QObject>

class SensorType
{
    public:
    // first constructor is to be removed
    explicit SensorType(int id, QString name = "", QString dbTableName = "", QString dllName = "", int dbColumnCount = 0);
    explicit SensorType(QString name, QString dbTableName = "", int dbColumnCount = 0);
    /// getters
    int getId() const;
    QString getName() const;
    QString getDbTableName() const;
    QString getDllName() const;

    /// setters
    void setId(int id);
    void setName(QString name);
    void setDbTableName(QString tn);
    void setDllName(QString dn);

    private:
        int id;
        QString m_name;
        QString m_dbTableName;
        QString m_dllName;
        int m_dbColumnCount;
};

#endif // SENSORTYPE_H
