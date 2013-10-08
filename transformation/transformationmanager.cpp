#include "transformationmanager.h"

TransformationManager* TransformationManager::m_instance = NULL;

TransformationManager *TransformationManager::instance()
{
    if(!m_instance){
        m_instance = new TransformationManager();
    }
    return m_instance;
}

QList<TransformationBaseClass *> TransformationManager::getTransformations() const
{
    return m_transformations.values();
}

TransformationBaseClass *TransformationManager::getTransformation(const QString &name) const
{
    return m_transformations.value(name);
}

void TransformationManager::assignTransformationToSensor(const QString &sensor, const QString &transformationName)
{
    if(m_assignedTransformations.contains(sensor)){
        if(m_assignedTransformations[sensor]){
            delete m_assignedTransformations[sensor];
        }
        if(m_transformations.contains(transformationName)){
            m_assignedTransformations[sensor] = m_transformations[transformationName]->newInstance();
        }else{
            m_assignedTransformations.remove(sensor);
        }
    }
}

TransformationBaseClass *TransformationManager::sensorTransformation(const QString &sensor)
{
    if(m_assignedTransformations[sensor]){
        return m_assignedTransformations[sensor];
    }
    return NULL;
}

int TransformationManager::load(QString path)
{
    if(path.isEmpty()){
        path = QDir::currentPath() + "/lib";
    }
    QDir dir(path);
    dir.setFilter(QDir::NoSymLinks | QDir::Files | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Name);
    QStringList nameFilters;
    nameFilters << "*.dll" << "*.DLL";
    dir.setNameFilters(nameFilters);

    QStringList list = dir.entryList();
    qDebug() << "TransformationManager::load(" << dir.path() << "):";
    int c = 0;
    for(int i=0; i<list.size(); ++i){
        QLibrary library(dir.path() + "/"+ list[i]);
        qDebug() << "Try to load lib: " << (dir.path() + "/"+ list[i]);
        bool okLoad = library.load(); // check load DLL file successful or not
        if(okLoad){
            qDebug() << "Loading succed";
            typedef TransformationBaseClass* (*TransformGetInstanceFunction)();
            TransformGetInstanceFunction trsf = (TransformGetInstanceFunction) library.resolve("getTransformationInstance");
            if (trsf){
                TransformationBaseClass* instance = trsf();
                QString name = instance->getTransformationDefinition().name;
                m_transformations.insert(name, instance);
                ++c;
            }
        }else{
            qDebug() << "Loading failed";
        }
    }

    return c;
}

bool TransformationManager::unload(QString &name)
{
    TransformationBaseClass *pt = m_transformations.value(name);
    m_transformations.remove(name);
    if(pt){
        delete pt;
        return true;
    }
    return false;
}

int TransformationManager::unloadAll()
{
    int count = 0;
    QStringList keys = m_transformations.keys();
    for(int i=0;i<keys.size(); ++i){
        TransformationBaseClass *pt = m_transformations.value(keys[i]);
        count += m_transformations.remove(keys[i]);
        if(pt){
            delete pt;
        }
    }
    return count;
}

TransformationManager::TransformationManager()
{
}

TransformationManager::~TransformationManager()
{
    qDeleteAll(m_transformations);
    qDeleteAll(m_assignedTransformations);
}
