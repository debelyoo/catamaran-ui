#ifndef SENSORTYPE_H
#define SENSORTYPE_H

#include <QObject>

class SensorType
{
public:
    explicit SensorType(const QString &name = "");
    /// getters
    const QString &name() const;

    /// setters
    void setName(QString name);

private:
    QString m_name;

};

#endif // SENSORTYPE_H
