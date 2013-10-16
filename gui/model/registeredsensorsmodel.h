#ifndef REGISTEREDSENSORSMODEL_H
#define REGISTEREDSENSORSMODEL_H

#include <QAbstractTableModel>
#include "registeredsensoritem.h"

class RegisteredSensorsModel : public QAbstractTableModel
{
	Q_OBJECT
public:
    typedef enum {
        AddressCol = 0,
        NameEditCol = 1,
        TransfCol = 2,
        ConfigCol = 3,
        StreamCol = 4,
        RecordCol = 5,
        DeleteCol = 6

    } ColumnDesc;

    explicit RegisteredSensorsModel(QObject *parent = 0);
    ~RegisteredSensorsModel();

    class Proxy : public QSortFilterProxyModel{
        // QSortFilterProxyModel interface
    protected:
        bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    };

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    void addSensor(RegisteredSensorItem *newItem, RegisteredSensorItem *parent = NULL);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
private:
    RegisteredSensorItem *getItem(const QModelIndex &index) const;
    int m_nColumn;
    QVector<QVariant> m_headers;
    QList<RegisteredSensorItem *> m_items;


};

#endif // REGISTEREDSENSORSMODEL_H
