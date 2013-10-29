#include "dataPlot.h"

DataPlot::DataPlot(QWidget *parent, QList<Sensor*> stp, int plotIndex) :
    QCustomPlot(parent),
    m_availableGraphIndex()
{
    m_availableGraphIndex.append(0);
    m_dbManager = DatabaseManager::instance();
    m_sensorsToPlot = stp;
    m_timeWindow = 15 * 60; // 15 minutes in seconds
    double now = QDateTime::currentDateTime().toTime_t();
    int fromTs = now - m_timeWindow;
    m_minValue = 99;
    m_maxValue = 0;
    m_plotIndex = plotIndex;

//    for (int gi=0; gi<m_sensorsToPlot.size(); ++gi)
//    {
//        this->addGraph();
//        QPen pen;
//        int hueOffset = 30; // the offset for the first color (30° -> orange)
//        pen.setColor(QColor::fromHsl(hueOffset + (25*gi), 255, 150));
//        //pen.setColor(QColor(255/4.0*gi, 160, 50, 200));
//        this->graph(gi)->setLineStyle(QCPGraph::lsLine);
//        this->graph(gi)->setPen(pen);

//        this->graph(gi)->setName(m_sensorsToPlot[gi]->name());

        // get data
//        QPair< QVector<double>, QVector<double> > data = m_dbManager->getData(m_sensorsToPlot[gi], fromTs);
//        this->graph(gi)->setData(data.first, data.second);
//        updateMinMaxValues(data.second);
//        this->graph(gi)->rescaleValueAxis(true);
//    }
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
    this->yAxis->setRange(m_minValue, m_maxValue);
    // show legend:
    this->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft);
    this->legend->setVisible(true);

    m_configBt = new QPushButton("Edit", this);
    m_configBt->setGeometry(5,5,40,20);
    m_configBt->show();
    connect(m_configBt, SIGNAL(clicked()), this, SLOT(on_editClicked()));
}

DataPlot::~DataPlot()
{
    delete m_configBt;
}

void DataPlot::updatePlot()
{
    //qDebug() << "DataPlot.updatePlot()";
    double now = QDateTime::currentDateTime().toTime_t();
    int fromTsAxis = now - m_timeWindow;
    int fromTs = now - 1; // time window of 1 second
    //qDebug() << "Range axis Y: " << minValue << " -> " << maxValue;
    foreach(Sensor * s, m_sensorToGraphHash.keys())
    {
        QCPGraph *graph = m_sensorToGraphHash[s];
        // get data for sensor
        QPair< QVector<double>, QVector<double> > data = m_dbManager->getData(s, fromTs);
        if (data.first.size() > 0)
        {
            graph->removeData(0, fromTsAxis); // remove old data
            graph->addData(data.first, data.second); // add data from the last second
            updateMinMaxValues(data.second);
        }
    }
    // set axis ranges to show all data:
    this->xAxis->setRange(fromTsAxis, now);
    this->yAxis->setRange(m_minValue, m_maxValue);
    // redraw plot
    this->replot();
}

void DataPlot::updateMinMaxValues(QVector<double> values)
{
    if (!values.isEmpty()) {
        QVector<double> vVal = values; // copy vector to sort it
        qSort(vVal); // sort values from smallest to highest
        if (vVal.first() < m_minValue){
            m_minValue = vVal.first();
            m_minValue *= (m_minValue < 0)?1.1:0.9;
        }
        if (vVal.last() > m_maxValue){
            m_maxValue = vVal.last();
            m_maxValue *= (m_maxValue > 0)?1.1:0.9;
        }
    }
}

/// for test only
QPair< QVector<double>, QVector<double> > DataPlot::getData(int gi)
{
    // generate random walk data:
    QVector<double> time(250), value(250);
    double now = QDateTime::currentDateTime().toTime_t(); // seconds
    int fromTs = now - m_timeWindow;
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



void DataPlot::resizeEvent(QResizeEvent *e)
{
    QCustomPlot::resizeEvent(e);
    QRect rect = m_configBt->geometry();
    rect.moveTo(rect.x(), this->height()-rect.height()-5);
    m_configBt->setGeometry(rect);
}

void DataPlot::addSensorToPlot(Sensor *sensor)
{
    int index = nextGraphIndex();

    QCPGraph * graph = this->addGraph();
    graph->setProperty("index", index);
    m_sensorToGraphHash[sensor] = graph;

    QPen pen;
    int hueOffset = 30; // the offset for the first color (30° -> orange)
    pen.setColor(QColor::fromHsl(hueOffset + (25*index), 255, 150));
    //pen.setColor(QColor(255/4.0*gi, 160, 50, 200));
    graph->setLineStyle(QCPGraph::lsLine);
    graph->setPen(pen);

    graph->setName(sensor->name());

    // get data
    //QPair< QVector<double>, QVector<double> > data = m_dbManager->getData(sensor, fromTs);
    //graph->setData(data.first, data.second);
    //updateMinMaxValues(data.second);
    //graph->rescaleValueAxis(true);
}

void DataPlot::removeSensorFromPlot(Sensor *sensor)
{
    QCPGraph *graph = m_sensorToGraphHash[sensor];
    m_availableGraphIndex.append(graph->property("index").toInt());
    removeGraph(graph);
    m_sensorToGraphHash.remove(sensor);
}

void DataPlot::updateSensorList()
{
    QList<Sensor *> oldSensorList = m_sensorsToPlot;
    m_sensorsToPlot = SensorConfig::instance()->sensorForPlot(m_plotIndex);
    // Remove obsolete graphs
    foreach(Sensor *s, oldSensorList){
        if(!m_sensorsToPlot.contains(s)){
            removeSensorFromPlot(s);
            if(m_sensorToGraphHash.contains(s)){
                removeSensorFromPlot(s);
            }
        }
    }
    // Add new graphs
    foreach(Sensor *s, m_sensorsToPlot){
        if(!oldSensorList.contains(s)){
            addSensorToPlot(s);
        }
    }
    updatePlot();
}

int DataPlot::nextGraphIndex()
{
    if(m_availableGraphIndex.isEmpty()){
        m_availableGraphIndex.append(1);
        return 0;
    }
    int index = m_availableGraphIndex.dequeue();
    if(m_availableGraphIndex.isEmpty()){
        m_availableGraphIndex.append(index+1);
    }
    return index;
}

void DataPlot::on_editClicked()
{
    AddToPlotWidget * widget = new AddToPlotWidget(m_plotIndex);
    widget->show();
    connect(widget, SIGNAL(savedButtonClicked(bool)), this, SLOT(on_editWidgetSaved(bool)));
}

void DataPlot::on_editWidgetSaved(bool changed)
{
    if(changed){
        updateSensorList();
    }
}
