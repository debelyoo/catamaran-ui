#ifndef SENSORTYPE_H
#define SENSORTYPE_H

#include <QObject>
#include <QByteArray>

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

QDataStream &operator<< (QDataStream &stream, const SensorType &sensorType);
#endif // SENSORTYPE_H
