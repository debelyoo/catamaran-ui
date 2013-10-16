#include "registeredsensorsmodel.h"

RegisteredSensorsModel::RegisteredSensorsModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_nColumn(7),
    m_headers(),
    m_items()
{
    m_headers.append("Address");
    m_headers.append("Name");
    m_headers.append("Transformation");
    m_headers.append(" ");
    m_headers.append("Stream");
    m_headers.append("Record");
    m_headers.append(" ");
}

RegisteredSensorsModel::~RegisteredSensorsModel()
{
    qDeleteAll(m_items);
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

    if (index.column() == StreamCol || index.column() == RecordCol)
    {
        returnFlags |= Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
    }
    if (index.column() == NameEditCol){
        returnFlags |= Qt::ItemIsEditable;
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
