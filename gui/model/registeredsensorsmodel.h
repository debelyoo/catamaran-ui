#ifndef REGISTEREDSENSORSMODEL_H
#define REGISTEREDSENSORSMODEL_H

#include <QAbstractTableModel>
#include "registeredsensoritem.h"

class RegisteredSensorsModel : public QAbstractTableModel
{
	Q_OBJECT
public:
    explicit RegisteredSensorsModel(QObject *parent = 0);
    ~RegisteredSensorsModel();

signals:

public slots:


    // QAbstractItemModel interface
public:
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    void addSensor(RegisteredSensorItem *newItem, RegisteredSensorItem *parent = NULL);

    //bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    //bool insertRows(int row, int count, const QModelIndex &parent);
    //bool removeRows(int row, int count, const QModelIndex &parent);
    //bool setData(const QModelIndex &index, const QVariant &value, int role);
private:
    RegisteredSensorItem *getItem(const QModelIndex &index) const;
    int m_nColumn;
    QVector<QVariant> m_headers;
    QList<RegisteredSensorItem *> m_items;
};

#endif // REGISTEREDSENSORSMODEL_H
