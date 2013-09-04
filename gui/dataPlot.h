#ifndef DATAPLOT_H
#define DATAPLOT_H

#include "qcustomplot.h"
#include "model/sensor.h"

class DataPlot : public QCustomPlot
{
    Q_OBJECT
public:
    explicit DataPlot(QWidget *parent = 0, QVector<Sensor*> stp = QVector<Sensor*>());
    ~DataPlot();

signals:

public slots:
    void updatePlot();

private:
    //int nbGraphsInPlot;
    QVector<Sensor*> sensorsToPlot;
    QPair< QVector<double>, QVector<double> >* getData(int gi);

};

#endif // DATAPLOT_H
