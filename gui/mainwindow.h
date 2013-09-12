#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "communication/server.h"
#include "util/fileHelper.h"
#include "util/coordinateHelper.h"
#include "dataPlot.h"
#include "mouseClickHandler.h"
#include <QMainWindow>
#include <QGridLayout>
#include <QPushButton>
#include <QTimer>
#include <QGraphicsEllipseItem>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void sendEngineCommand();
    void drawWayPointOnMap(QPoint);
    
public slots:
    void addStatusText(QString);
    void updateLeftSpinBox();
    void updateRightSpinBox();
    void updateSpeedSpinBox();
    void updateDirectionSpinBox();
    void on_navModeChanged(int);
    void on_sliderPressed();
    void on_sliderReleased();
    void on_speedValueChanged(int);
    void on_directionValueChanged(int);
    void zoomIn();
    void zoomOut();
    void setupMatrix();
    void drawPointOnMap(double, double);
    void on_removeWpClicked();
    void on_clearWpClicked();
    /// config update
    void on_addressValueChanged(QString, int);
    void on_nameValueChanged(QString, int);
    void on_typeValueChanged(int, int);
    void on_displayValueChanged(int, int);
    void on_recordValueChanged(bool, int);
    void on_streamValueChanged(bool, int);
    void on_filenameValueChanged(QString, int);
    void on_saveConfigClicked();
    void on_graphNbValueChanged(int);

protected:
    void keyPressEvent(QKeyEvent* event);

private:
    Ui::MainWindow *ui;
    Server *s;
    SensorConfig* sensorConfig; // singleton
    FileHelper* fileHelper;
    ByteArrayConverter* converter; // singleton
    CoordinateHelper* coordinateHelper;
    bool sliderIsMoving;
    int correctEngineCommandValue(int val);
    void updateLeftRightSliders();
    void sendLeftEngineCommand();
    void sendRightEngineCommand();
    int zoomStep;
    bool viewportIsMoving;
    /// plots panel
    void clearPlotsPanel();
    void createPlotsPanel();
    QWidget* createPlot(int xPos, int yPos, int width, int height);
    QWidget* createPlotByDate(int plotIndex, QRect geometry);
    /// configuration panel
    void clearAddressesConfigPanel();
    void createConfigurationPanel();
    void createAddressesConfigPanel();
    void createLabelLine(QGridLayout* l);
    void createAddressFormRow(QGridLayout* l, int rowIndex, Sensor* s);
    QWidget* createSpacedWidget(QWidget *w, int spaceBefore, int spaceAfter);
    QPushButton* saveBtn;
    void changeSaveBtnColor(QString cssColor);
    struct PointOnMap {
        QPoint p;
        QGraphicsEllipseItem* circle;
        QGraphicsLineItem* line;
    };
    QList<PointOnMap> wayPoints;
    void removeLastWaypoint();
    void sendWaypointCommand(quint8 command, QList<QPointF> points);
};

#endif // MAINWINDOW_H
