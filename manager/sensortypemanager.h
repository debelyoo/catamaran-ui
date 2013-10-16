#ifndef SENSORTYPEMANAGER_H
#define SENSORTYPEMANAGER_H

#include <QStringList>
#include <QMap>
#include "model/sensorType.h"

class SensorTypeManager
{
public:
    static SensorTypeManager *instance();

    const QStringList &list() const;
    const SensorType *type(const QString & name) const;

    //void addType(SensorType *);
    bool createType(const QString &name, const QString &dbTableName, int dbColumnCount = 0);
    bool deleteType(const QString &name);
    bool updateType(const QString &name, const QString &dbTableName, int dbColumnCount = 0);

    //int loadFromfile(const QString &path);

private:
    Q_DISABLE_COPY(SensorTypeManager)
    SensorTypeManager();

    static SensorTypeManager *s_instance;

    QMap<QString, SensorType *> m_types;
};

#endif // SENSORTYPEMANAGER_H
