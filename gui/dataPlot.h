#ifndef DATAPLOT_H
#define DATAPLOT_H

#include "qcustomplot.h"

class DataPlot : public QCustomPlot
{
    Q_OBJECT
public:
    explicit DataPlot(QWidget *parent = 0);
    ~DataPlot();

signals:

public slots:
    void updatePlot();

private:
    QPair< QVector<int>, QVector<int> > getData();

};

#endif // DATAPLOT_H
