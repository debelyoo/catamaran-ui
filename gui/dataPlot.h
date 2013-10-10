#ifndef DATAPLOT_H
#define DATAPLOT_H

#include "qcustomplot.h"
#include "model/sensor.h"
#include "util/databaseManager.h"

class DataPlot : public QCustomPlot
{
    Q_OBJECT
public:
    explicit DataPlot(QWidget *parent = 0, QList<Sensor*> stp = QList<Sensor*>());
    ~DataPlot();

signals:

public slots:
    void updatePlot();

protected:
    //void wheelEvent(QWheelEvent *event);

private:
    DatabaseManager* dbManager;
    QList<Sensor*> sensorsToPlot;
    QPair< QVector<double>, QVector<double> > getData(int gi);
    void updateMinMaxValues(QVector<double> vVal);
    int timeWindow;
    int minValue, maxValue;
};

#endif // DATAPLOT_H
