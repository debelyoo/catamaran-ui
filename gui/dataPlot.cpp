#include "dataPlot.h"

DataPlot::DataPlot(QWidget *parent, QList<Sensor*> stp) :
    QCustomPlot(parent)
{
    dbManager = DatabaseManager::instance();
    sensorsToPlot = stp;
    timeWindow = 15 * 60; // 15 minutes in seconds
    double now = QDateTime::currentDateTime().toTime_t();
    int fromTs = now - timeWindow;
    minValue = 99;
    maxValue = 0;

    for (int gi=0; gi<sensorsToPlot.size(); ++gi)
    {
        this->addGraph();
        QPen pen;
        //before: 0, 0, 255, 200
        pen.setColor(QColor(255/4.0*gi, 160, 50, 200));
        this->graph(gi)->setLineStyle(QCPGraph::lsLine);
        this->graph(gi)->setPen(pen);
        this->graph(gi)->setName(sensorsToPlot[gi]->getName());
        //customPlot->graph()->setBrush(QBrush(QColor(255/4.0*gi,160,50,150)));
        // get data
        QPair< QVector<double>, QVector<double> > data = dbManager->getData(sensorsToPlot[gi], fromTs);
        this->graph(gi)->setData(data.first, data.second);
        this->graph(gi)->rescaleValueAxis(true);
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
    this->xAxis->setTickStep(1 * 60); // one minute in seconds
    this->xAxis->setSubTickCount(3);
    // apply manual tick and tick label for left axis:
    //this->yAxis->setAutoTicks(false);
    //this->yAxis->setAutoTickLabels(false);
    //this->yAxis->setTickVector(QVector<double>() << 5 << 55);
    //this->yAxis->setTickVectorLabels(QVector<QString>() << "Not so\nhigh" << "Very\nhigh");
    // set axis labels:
    this->xAxis->setLabel("Date");
    //this->yAxis->setLabel("Random wobbly lines value");
    // make top and right axes visible but without ticks and labels:
    this->xAxis2->setVisible(true);
    this->yAxis2->setVisible(true);
    this->xAxis2->setTicks(false);
    this->yAxis2->setTicks(false);
    this->xAxis2->setTickLabels(false);
    this->yAxis2->setTickLabels(false);
    // set axis ranges to show all data:
    this->xAxis->setRange(fromTs, now);
    //this->yAxis->setRange(0, 30);
    // show legend:
    this->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft);
    this->legend->setVisible(true);
}

DataPlot::~DataPlot()
{

}

void DataPlot::updatePlot()
{
    //qDebug() << "DataPlot.updatePlot()";
    double now = QDateTime::currentDateTime().toTime_t();
    //int fromTs = now - timeWindow;
    int fromTs = now - 1; // time window of 1 second
    //qDebug() << "Range: "+ QString::number(fromTs)+" -> "+ QString::number(now);
    for (int i=0; i<sensorsToPlot.size(); ++i)
    {
        // get data for sensor
        QPair< QVector<double>, QVector<double> > data = dbManager->getData(sensorsToPlot[i], fromTs);
        //QPair< QVector<double>, QVector<double> >* data = getData(i);
        //qDebug() << data.first.count();
        if (data.first.size() > 0)
        {
            //this->graph(i)->setData(data.first, data.second);
            this->graph(i)->removeData(0, fromTs); // remove old data
            this->graph(i)->addData(data.first, data.second); // add data from the last second
            QVector<double> vVal = data.second;
            qSort(vVal);
            if (vVal.first() < minValue)
                minValue = vVal.first() - (vVal.first() * 0.1);
            if (vVal.last() > maxValue)
                maxValue = vVal.last() + (vVal.last() * 0.1);
            //this->graph(i)->rescaleValueAxis(false);
        }
    }
    // set axis ranges to show all data:
    this->xAxis->setRange(fromTs, now);
    this->yAxis->setRange(minValue, maxValue);
    // redraw plot
    this->replot();
}

/// for test only
QPair< QVector<double>, QVector<double> > DataPlot::getData(int gi)
{
    // generate random walk data:
    QVector<double> time(250), value(250);
    double now = QDateTime::currentDateTime().toTime_t(); // seconds
    int fromTs = now - timeWindow;
    //double now = QDateTime::currentDateTime().toMSecsSinceEpoch() / (double)1000; // milliseconds
    for (int i=0; i<250; ++i)
    {
      time[i] = fromTs + i; // every sec
      if (i == 0)
        value[i] = (i/50.0+1)*(rand()/(double)RAND_MAX-0.5);
      else
        value[i] = fabs(value[i-1])*(1+0.02/4.0*(4-gi)) + (i/50.0+1)*(rand()/(double)RAND_MAX-0.5);
    }
    QPair< QVector<double>, QVector<double> > data = QPair< QVector<double>, QVector<double> >(time, value);
    return data;
}

