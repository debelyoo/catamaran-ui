#include "registeredsensorsmodel.h"

RegisteredSensorsModel::RegisteredSensorsModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_nColumn(3),
    m_headers(),
    m_items()
{
    m_headers.append("Name");
    m_headers.append("Transformation");
    m_headers.append(" ");

    //emit headerDataChanged(Qt::Horizontal, 0, m_nColumn-1);
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
    if (!index.isValid() || index.column() >= m_nColumn)
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();
    RegisteredSensorItem *item = getItem(index);
    switch(index.column()){
    case 0:
    case 1:
        if(item){
            return item->data(index.column());
        }else{
            return QVariant();
        }
    case 2:
        return QVariant("Configure");
    case 3:
    default:
        break;
    }
    return QVariant();
}

void RegisteredSensorsModel::addSensor(RegisteredSensorItem *newItem, RegisteredSensorItem *parent)
{
    //() << "Add Sensor : " << m_items.count();
    beginInsertRows(QModelIndex(), m_items.count(), m_items.count());;
    if(parent){
        parent->addChild(newItem);
    }

    m_items.append(newItem);
    endInsertRows();
    //QModelIndex index = createIndex(m_items.size()-1, 0, newItem);
    //QModelIndex index2 = createIndex(m_items.size()-1, m_nColumn, newItem);
    //emit dataChanged(index, index2);
}

RegisteredSensorItem *RegisteredSensorsModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        RegisteredSensorItem *item = static_cast<RegisteredSensorItem*>(index.internalPointer());
        //RegisteredSensorItem *item = m_items[index.row()];
        //() << "getItem : pointer = " << ((int) item);
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

bool RegisteredSensorsModel::Proxy::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QString lsId, rsId;
    lsId = static_cast<RegisteredSensorItem *>(left.internalPointer())->sortId();
    rsId = static_cast<RegisteredSensorItem *>(right.internalPointer())->sortId();
    int len = qMin(lsId.length(), rsId.length());
    for(int i=0;i<len;++i){
        //() << "sort : " << lsId[i] << " < " << rsId[i] << "?";
        if(lsId[i] < rsId[i]){
            return false;
        }else if(lsId[i] > rsId[i]){
            return true;
        }
    }
    return false;
}

/*
bool RegisteredSensorsModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position+rows-1);

    for (int row=0; row < rows; row++) {
        m_items.insert(position, NULL);
    }

    endInsertRows();
    return true;
}

bool RegisteredSensorsModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    for (int row=0; row < rows; ++row) {
        m_items.removeAt(position);
    }

    endRemoveRows();
    return true;
}
*/


