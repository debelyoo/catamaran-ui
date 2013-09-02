#include "dataPlot.h"

DataPlot::DataPlot(QWidget *parent) :
    QCustomPlot(parent)
{

}

DataPlot::~DataPlot()
{

}

void DataPlot::updatePlot()
{
    // get data

    // redraw plot
    replot();
}

QPair< QVector<int>, QVector<int> > DataPlot::getData()
{

}

