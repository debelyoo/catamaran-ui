#ifndef SENSORTYPEMANAGER_H
#define SENSORTYPEMANAGER_H

#include <QStringList>
#include <QMap>
#include "model/sensorType.h"

class SensorTypeManager
{
public:
    static SensorTypeManager *instance();

    const QStringList list() const;
    const QStringList &sortedList() const;
    const SensorType *type(const QString & name) const;

    //void addType(SensorType *);
    bool createType(const QString &name);
    bool deleteType(const QString &name);
    bool updateType(const QString &name);

    //int loadFromfile(const QString &path);
    friend QDataStream &operator>> (QDataStream &stream, SensorTypeManager &a);
    friend QDataStream &operator<< (QDataStream &stream, const SensorTypeManager &a);

private:
    Q_DISABLE_COPY(SensorTypeManager)
    SensorTypeManager();

    static SensorTypeManager *s_instance;

    QMap<QString, SensorType *> m_types;
    QStringList m_sortedList;

    void updateSortedList();
};

QDataStream &operator<< (QDataStream &stream, const SensorTypeManager &a);
QDataStream &operator>> (QDataStream &stream, SensorTypeManager &a);

#endif // SENSORTYPEMANAGER_H
