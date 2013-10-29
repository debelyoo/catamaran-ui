#ifndef DATAPLOT_H
#define DATAPLOT_H

#include <QPushButton>
#include "qcustomplot.h"
#include "model/sensor.h"
#include "util/database/databaseManager.h"
#include "gui/addtoplotwidget.h"

class DataPlot : public QCustomPlot
{
    Q_OBJECT
public:
    explicit DataPlot(QWidget *parent = 0, QList<Sensor*> stp = QList<Sensor*>(), int plotIndex = 0);
    ~DataPlot();

signals:

public slots:
    void updatePlot();

protected:
    //void wheelEvent(QWheelEvent *event);

private:
    DatabaseManager *m_dbManager;
    QList<Sensor *> m_sensorsToPlot;
    QHash<Sensor *, QCPGraph *> m_sensorToGraphHash;

    QPair< QVector<double>, QVector<double> > getData(int gi);
    void updateMinMaxValues(QVector<double> vVal);
    int m_timeWindow;
    int m_minValue, m_maxValue;

    // QWidget interface
protected:
    QPushButton *m_configBt;
    int m_plotIndex;
    QQueue<int> m_availableGraphIndex;

    void resizeEvent(QResizeEvent *);
    void addSensorToPlot(Sensor *sensor);
    void removeSensorFromPlot(Sensor *sensor);
    void updateSensorList();

    int nextGraphIndex();

private slots:
    void on_editClicked();
    void on_editWidgetSaved(bool changed);
};

#endif // DATAPLOT_H
