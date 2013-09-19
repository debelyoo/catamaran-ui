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

TransformationBaseClass *TransformationManager::getTransformation(const QString name) const
{
    return m_transformations.value(name);
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
            }
        }else{
            qDebug() << "Loading failed";
        }
    }
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
