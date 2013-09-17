#ifndef SENSORINPUTSMODEL_H
#define SENSORINPUTSMODEL_H

#include <QAbstractItemModel>
#include "sensorinputitem.h"

class SensorInputsModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	explicit SensorInputsModel(QObject *parent = 0);
    ~SensorInputsModel();

signals:

public slots:


    // QAbstractItemModel interface
public:
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role);

    Qt::ItemFlags flags(const QModelIndex &index) const;

    void addInput(SensorInputItem *newItem, SensorInputItem *parent = NULL);
private:
    SensorInputItem *m_rootItem;
    SensorInputItem *getItem(const QModelIndex &index) const;       
};

#endif // SENSORINPUTSMODEL_H
