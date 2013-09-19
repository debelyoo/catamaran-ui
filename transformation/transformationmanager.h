#ifndef TRANSFORMATIONMANAGER_H
#define TRANSFORMATIONMANAGER_H

#include <QList>
#include <QString>
#include <QHash>
#include <QDir>
#include <QLibrary>
#include "transformation/transformationbaseclass.h"

class TransformationManager
{
public:
    static TransformationManager *instance();

    QList<TransformationBaseClass *> getTransformations();
    int load(QString path = "");
    bool unload(QString &name);
    int unloadAll();

private:
    TransformationManager();

    static TransformationManager *m_instance;
    QHash<QString, TransformationBaseClass *> m_transformations;
};

#endif // TRANSFORMATIONMANAGER_H
