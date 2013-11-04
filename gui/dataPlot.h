#ifndef DATAPLOT_H
#define DATAPLOT_H

#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include "qcustomplot.h"
#include "model/sensor.h"
#include "util/database/databaseManager.h"
#include "gui/addtoplotwidget.h"

class DataPlot : public QCustomPlot
{
    Q_OBJECT
public:
    explicit DataPlot(int plotIndex, QWidget *parent = 0);
    int timeWindow() const;
    ~DataPlot();

signals:

public slots:
    void updatePlot(int deltaT = 1);
    void updateSensorList();

protected:
    //void wheelEvent(QWheelEvent *event);

private:
    DatabaseManager *m_dbManager;
    QList<Sensor *> m_sensorsToPlot;
    QHash<Sensor *, QCPGraph *> m_sensorToGraphHash;

    QPair< QVector<double>, QVector<double> > getData(int gi);
    void updateMinMaxValues(QVector<double> vVal, bool forceUpdate = false);
    int m_timeWindow;
    double m_minValue, m_maxValue;
    double m_yWindowPrc;

    // QWidget interface
protected:
    QWidget *m_uiContainer;

    int m_plotIndex;
    QQueue<int> m_availableGraphIndex;

    void resizeEvent(QResizeEvent *);
    void addSensorToPlot(Sensor *sensor);
    void removeSensorFromPlot(Sensor *sensor);
    //void updateSensorList();

    int nextGraphIndex();

    void resetMinMax();

private slots:
    void on_editClicked();
    void on_editWidgetSaved(bool changed);
    void on_yScaleChange(int scale);
};

#endif // DATAPLOT_H
