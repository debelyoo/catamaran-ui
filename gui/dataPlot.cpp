#include "dataPlot.h"

DataPlot::DataPlot(QWidget *parent, QList<Sensor*> stp) :
    QCustomPlot(parent)
{
    dbManager = DatabaseManager::instance();
    sensorsToPlot = stp;
    for (int gi=0; gi<sensorsToPlot.size(); ++gi)
    {
        this->addGraph();
        QPen pen;
        //before: 0, 0, 255, 200
        pen.setColor(QColor(255/4.0*gi, 160, 50, 200));
        this->graph()->setLineStyle(QCPGraph::lsLine);
        this->graph()->setPen(pen);
        //customPlot->graph()->setBrush(QBrush(QColor(255/4.0*gi,160,50,150)));
        // get data
        QPair< QVector<double>, QVector<double> >* data = getData(gi);
        this->graph()->setData(data->first, data->second);
    }
    // configure bottom axis to show date and time instead of number:
    this->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    //this->xAxis->setDateTimeFormat("MMMM\nyyyy");
    this->xAxis->setDateTimeFormat("HH:mm:ss");
    // set a more compact font size for bottom and left axis tick labels:
    this->xAxis->setTickLabelFont(QFont(QFont().family(), 8));
    this->yAxis->setTickLabelFont(QFont(QFont().family(), 8));
    // set a fixed tick-step to one tick per month:
    this->xAxis->setAutoTickStep(false);
    //this->xAxis->setTickStep(2628000); // one month in seconds
    this->xAxis->setTickStep(15); // in seconds
    this->xAxis->setSubTickCount(3);
    // apply manual tick and tick label for left axis:
    //this->yAxis->setAutoTicks(false);
    //this->yAxis->setAutoTickLabels(false);
    //this->yAxis->setTickVector(QVector<double>() << 5 << 55);
    //this->yAxis->setTickVectorLabels(QVector<QString>() << "Not so\nhigh" << "Very\nhigh");
    // set axis labels:
    this->xAxis->setLabel("Date");
    this->yAxis->setLabel("Random wobbly lines value");
    // make top and right axes visible but without ticks and labels:
    this->xAxis2->setVisible(true);
    this->yAxis2->setVisible(true);
    this->xAxis2->setTicks(false);
    this->yAxis2->setTicks(false);
    this->xAxis2->setTickLabels(false);
    this->yAxis2->setTickLabels(false);
    // show legend:
    this->legend->setVisible(true);
}

DataPlot::~DataPlot()
{

}

void DataPlot::updatePlot()
{
    //qDebug() << "DataPlot.updatePlot()";
    for (int i=0; i<sensorsToPlot.size(); ++i)
    {
        // get data for sensor
        QPair< QVector<double>, QVector<double> >* data = dbManager->getData(sensorsToPlot[i]);
        //QPair< QVector<double>, QVector<double> >* data = getData(gi);
        this->graph(i)->setData(data->first, data->second);
    }
    // set axis ranges to show all data:
    double now = QDateTime::currentDateTime().toTime_t();
    this->xAxis->setRange(now, now+249);
    this->yAxis->setRange(0, 60);
    // redraw plot
    this->replot();
}

QPair< QVector<double>, QVector<double> >* DataPlot::getData(int gi)
{
    // generate random walk data:
    QVector<double> time(250), value(250);
    double now = QDateTime::currentDateTime().toTime_t();
    for (int i=0; i<250; ++i)
    {
      time[i] = now + i; // every sec
      if (i == 0)
        value[i] = (i/50.0+1)*(rand()/(double)RAND_MAX-0.5);
      else
        value[i] = fabs(value[i-1])*(1+0.02/4.0*(4-gi)) + (i/50.0+1)*(rand()/(double)RAND_MAX-0.5);
    }
    QPair< QVector<double>, QVector<double> >* data = new QPair< QVector<double>, QVector<double> >(time, value);
    return data;
}

