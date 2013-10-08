#ifndef TRANSFORMATIONMANAGER_H
#define TRANSFORMATIONMANAGER_H

#include <QList>
#include <QString>
#include <QHash>
#include <QMap>
#include <QDir>
#include <QLibrary>
#include "transformation/transformationbaseclass.h"

class TransformationManager
{
public:
    static TransformationManager *instance();

    QList<TransformationBaseClass *> getTransformations() const;
    TransformationBaseClass* getTransformation(const QString &name) const;
    void assignTransformationToSensor(const QString &sensor, const QString &transformationName);
    TransformationBaseClass *sensorTransformation(const QString &sensor);
    int load(QString path = "");
    bool unload(QString &name);
    int unloadAll();

private:
    TransformationManager();
    ~TransformationManager();

    static TransformationManager *m_instance;
    QMap<QString, TransformationBaseClass *> m_transformations;
    QHash<QString, TransformationBaseClass *> m_assignedTransformations;
};

#endif // TRANSFORMATIONMANAGER_H
