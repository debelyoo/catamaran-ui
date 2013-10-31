#include "sensorinputsmodel.h"

SensorInputsModel::SensorInputsModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    m_rootItem = new SensorInputItem("Sensor Inputs");
}

SensorInputsModel::~SensorInputsModel()
{
    delete m_rootItem;
}

void SensorInputsModel::itemModified()
{
    const SensorInputItem *item = static_cast<const SensorInputItem *>(sender());
    //qDebug() << "item modified at row " << item->childNumber() << " of " << item->parent()->name();
    if(item){
        QModelIndex index = createIndex(item->childNumber(), 0, item->parent());
        emit dataChanged(index, index);
    }
    foreach(SensorInputItem * i, item->childs()){
        connect(i, SIGNAL(itemChanged()), SLOT(itemModified()));
    }
}


QModelIndex SensorInputsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0){
        return QModelIndex();
    }

    SensorInputItem *parentItem = getItem(parent);

    SensorInputItem *childItem = parentItem->child(row);
    if (childItem){
        return createIndex(row, column, childItem);
    }else{
        return QModelIndex();
    }
}

QModelIndex SensorInputsModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    SensorInputItem *childItem = getItem(child);
    SensorInputItem *parentItem = childItem->parent();

    if (parentItem == m_rootItem || !parentItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

int SensorInputsModel::rowCount(const QModelIndex &parent) const
{
    SensorInputItem *parentItem = getItem(parent);
    return parentItem->childCount();
}

int SensorInputsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}


QVariant SensorInputsModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid() || index.column() != 0)
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    SensorInputItem *item = getItem(index);

    return QVariant(item->name());

}

void SensorInputsModel::addInput(SensorInputItem *newItem, SensorInputItem *parent)
{
    if(!parent){
        parent = m_rootItem;
    }
    parent->addChild(newItem);
    connect(newItem, SIGNAL(itemChanged()), SLOT(itemModified()));

    QModelIndex index = createIndex(newItem->childNumber(), 0, newItem);
    emit dataChanged(index, index);
}

SensorInputItem *SensorInputsModel::getItem(const QString &address) const
{
    SensorAddress sa(address);
    SensorInputItem *item = m_rootItem;
    bool found = false;
    for(int level=0;level<sa.level(); ++level){
        found = false;
        foreach(SensorInputItem *i, item->childs()){
            if(i->address() == sa.parent(level)){
                item = i;
                found = true;
                break;
            }
        }
        if(!found){
            return NULL;
        }
    }
    found = false;
    foreach(SensorInputItem *i, item->childs()){
        if(i->address() == sa){
            item = i;
            found = true;
            break;
        }
    }
    if(!found){
        return NULL;
    }
    return item;
}

SensorInputItem *SensorInputsModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        SensorInputItem *item = static_cast<SensorInputItem*>(index.internalPointer());
        if (item) return item;
    }
    return m_rootItem;
}

QVariant SensorInputsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0)
        return QVariant(m_rootItem->name());
    return QVariant();
}

bool SensorInputsModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;
    m_rootItem->setName(value.toString());
    bool result = true;
    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}


Qt::ItemFlags SensorInputsModel::flags(const QModelIndex &index) const
{
    SensorInputItem *item = static_cast<SensorInputItem*>(index.internalPointer());
    Qt::ItemFlags flag = Qt::NoItemFlags;
    if(item->enabled()){
        flag |= Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    return flag;
}
