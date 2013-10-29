#include "registeredsensorsmodel.h"

RegisteredSensorsModel::RegisteredSensorsModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_nColumn(9),
    m_headers(),
    m_items()
{
    m_headers.append("Address");
    m_headers.append("Name");
    m_headers.append("Type");
    m_headers.append("Transformation");
    m_headers.append(" ");
    m_headers.append("Stream");
    m_headers.append("Record");
    m_headers.append("Lof file prefix");
    m_headers.append(" ");
}

RegisteredSensorsModel::~RegisteredSensorsModel()
{
    //qDeleteAll(m_items);
    while(m_items.count() > 0){
        foreach(RegisteredSensorItem *item, m_items){
            if(item->childCount() == 0){
                m_items.removeOne(item);
                delete item;
            }
        }
    }
}


QModelIndex RegisteredSensorsModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (column < m_nColumn && row < m_items.count() && row >= 0){
        return createIndex(row, column, m_items[row]);
    }
    return QModelIndex();
}

int RegisteredSensorsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_items.size();
}

int RegisteredSensorsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_nColumn;
}


QVariant RegisteredSensorsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.column() >= m_nColumn){
        return QVariant::Invalid;
    }
    RegisteredSensorItem *item = getItem(index);
    if(!item){
        return QVariant::Invalid;
    }
    switch(role){
    case Qt::DisplayRole:
    case Qt::EditRole:
    {
        switch(index.column()){
        case AddressCol:
            return item->sensor()->address();
        case NameEditCol:
            return item->sensor()->name();
        case PrefixLogCol:
            return item->sensor()->logFilePrefix();
        case TypeCol:
            if(item->sensor() && item->sensor()->type()){
                return QVariant(item->sensor()->type()->name());
            }
            return QVariant("Unknown");
        case TransfCol:
            if(!item->transformation()){
                return QVariant("None");
            }
            return item->transformation()->getTransformationDefinition().name;
        case ConfigCol:
            return QVariant("Configure");
        case StreamCol:
            return item->sensor()->stream();
        case RecordCol:
            return item->sensor()->record();
        default:
            return QVariant::Invalid;
        }
    }
        break;
    case Qt::CheckStateRole:
    {
        switch(index.column()){
        case StreamCol:
            return item->sensor()->stream()?Qt::Checked:Qt::Unchecked;
        case RecordCol:
            return item->sensor()->record()?Qt::Checked:Qt::Unchecked;
        default:
            break;
        }
        break;
    }
    case Qt::DecorationRole:
        switch(index.column()){
        case DeleteCol:
            if(index.flags() & Qt::ItemIsEnabled){
                return QIcon(":/images/ressources/style/delete-16.png");
            }else{
                return QVariant::Invalid;
            }
            break;
        default:
            break;
        }
        break;
    case Qt::SizeHintRole:
        switch(index.column()){
        case DeleteCol:
            return QSize(16, 16);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return QVariant();
}

void RegisteredSensorsModel::addSensor(RegisteredSensorItem *newItem, RegisteredSensorItem *parent)
{
    beginInsertRows(QModelIndex(), m_items.count(), m_items.count());;
    if(parent){
        parent->addChild(newItem);
    }
    m_items.append(newItem);
    endInsertRows();
}

void RegisteredSensorsModel::removeSensor(QModelIndex index)
{
    RegisteredSensorItem *item = getItem(index);
    if(item){
        beginRemoveRows(QModelIndex(), index.row(), index.row());
        m_items.removeOne(item);
        delete item;
        endRemoveRows();
    }
}

QList<RegisteredSensorItem *> RegisteredSensorsModel::items()
{
    return m_items;
}

RegisteredSensorItem *RegisteredSensorsModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        RegisteredSensorItem *item = static_cast<RegisteredSensorItem*>(index.internalPointer());

        if (item) return item;
    }
    return NULL;
}

QVariant RegisteredSensorsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(role)
    if(role == Qt::DisplayRole && orientation == Qt::Horizontal){
        if(section < m_nColumn){
            return m_headers[section];
        }
    }
    return QVariant();
}

Qt::ItemFlags RegisteredSensorsModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags returnFlags = QAbstractTableModel::flags(index);

    if (index.column() == StreamCol)
    {
        returnFlags |= Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
    }else if(index.column() == RecordCol)
    {
        returnFlags |= Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
        QModelIndex m = createIndex(index.row(), TypeCol, index.internalPointer());
        if(data(m, Qt::DisplayRole).toString() == "Unknown"){
            returnFlags &= ~Qt::ItemIsEnabled;
        }
    }
    if (index.column() == NameEditCol || index.column() == PrefixLogCol){
        returnFlags |= Qt::ItemIsEditable;
    }
    if(index.column() == DeleteCol){
        RegisteredSensorItem * item = getItem(index);
        if(item && item->childCount() > 0){
            returnFlags &= ~Qt::ItemIsEnabled;
        }
    }

    return returnFlags;
}


bool RegisteredSensorsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    RegisteredSensorItem *item = getItem(index);
    if(item){
       if (role == Qt::CheckStateRole)
       {
           switch(index.column()){
           case StreamCol:
               item->setStream(value.toBool());
               emit dataChanged(index, index);
               return true;
           case RecordCol:
               item->setRecord(value.toBool());
               emit dataChanged(index, index);
               return true;
           default:
               break;
           }
       }
       if(role == Qt::EditRole){
           switch(index.column()){
           case NameEditCol:
               item->setName(value.toString());
               emit dataChanged(index, index);
               return true;
           case PrefixLogCol:
               item->setLogFilePrefix(value.toString());
               emit dataChanged(index, index);
               return true;
           case TypeCol:
               item->setType(value.toString());
               emit dataChanged(index, index);
               return true;
           default:
               break;
           }
       }
   }

    return false;
}

bool RegisteredSensorsModel::Proxy::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QString lsId, rsId;
    lsId = static_cast<RegisteredSensorItem *>(left.internalPointer())->sortId();
    rsId = static_cast<RegisteredSensorItem *>(right.internalPointer())->sortId();
    int len = qMin(lsId.length(), rsId.length());
    for(int i=0;i<len;++i){
        if(lsId[i] < rsId[i]){
            return false;
        }else if(lsId[i] > rsId[i]){
            return true;
        }
    }
    return false;
}
