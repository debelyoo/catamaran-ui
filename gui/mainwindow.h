#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "communication/server.h"
#include "util/fileHelper.h"
#include "util/coordinateHelper.h"
#include "util/criobytearray.h"
#include "dataPlot.h"
#include "mouseClickHandler.h"
#include <QMainWindow>
#include <QGridLayout>
#include <QPushButton>
#include <QTimer>
#include <QGraphicsEllipseItem>
#include "gui/model/sensorinputsmodel.h"
#include "gui/model/registeredsensorsmodel.h"
#include "gui/delegate/registeredSensorsDelegate.h"
#include "gui/delegate/comboboxdelegate.h"
#include "model/compactrio.h"
#include "communication/dataExporter.h"


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
    void drawWayPointOnMap(QPointF, bool);
    
public slots:
    void on_navSysStart();
    void addStatusText(QString);
    void addCrioStatusText(QString);
    void updateLeftSpinBox();
    void updateRightSpinBox();
    void updateSpeedSpinBox();
    void updateDirectionSpinBox();
    void on_navModeChanged(int);
    void on_sliderPressed();
    void on_sliderReleased();
    void on_speedValueChanged(int);
    void on_directionValueChanged(int);
    void setSliderIsMoving(bool);
    void on_tabChanged(int);

    /// Map
    void zoomIn();
    void zoomOut();
    void setupMatrix();
    void drawPointOnMap(double, double);
    void on_removeWpClicked();
    void on_clearWpClicked();
    void on_cleanGPSClicked();
    void on_graphWheelEvent(QWheelEvent*);

    /// server event
    void on_newConnection();

    /// config update
    void on_graphNbValueChanged(int);

    /// sensor transformation and config tables events
    void on_availableSensorsValueChanged();
    void on_addSensorFlClicked();
    void on_registeredSensorButtonClick(QModelIndex&);
    void on_sensorConfigChanged();
    void on_registeredSensorClicked(QModelIndex index);

    /// Honk and Light
    void on_honkButtonPressed();
    void on_honkButtonReleased();
    void on_lightCheckBoxChange();

    /// Navigation System config
    void on_updateNSBtnClick();
    void on_nsValueChange();
    void on_getNSCongifBtnClick();
    void on_defaultNSConfigClick();
    void on_loadWPBtnClicked();

    /// CompactRio State changed
    void on_crioPositionChanged();
    void on_crioSpeedChanged();
    void on_crioHeadingChanged();

    void on_saveConfig();
    void on_loadConfig();

protected slots:
    void on_engineValueAutoUpdate();

    /// export
    void on_exportBtnClicked();
    void on_removeMissionBtnClicked();
    void on_missionSelectedChanged(QItemSelection);
    void on_backendAddressValueChanged(QString);
    void on_pingRequestDone(int);
    void on_sensorTypeRequestDone(int, QList<QString>);

protected:
    void keyPressEvent(QKeyEvent* event);

private:
    Ui::MainWindow *ui;
    Server *server;
    DataExporter* dataExporter;
    HttpRequester* httpRequester;
    DatabaseManager* dbManager;
    SensorConfig* sensorConfig; // singleton
    FileHelper* fileHelper;
    CoordinateHelper* coordinateHelper;
    CompactRio *compactRio;

    bool sliderIsMoving;
    int previousSpeedValue, previousDirectionValue;
    int correctEngineCommandValue(int val);
    void updateLeftRightSliders();
    void updateSpeedDirectionSliders(int left, int right);
    void sendLeftEngineCommand();
    void sendRightEngineCommand();
    int zoomStep;
    bool viewportIsMoving;
    /// Sensor load config
    void buildConfigSensorsView();
    /// plots panel
    void clearPlotsPanel();
    void createPlotsPanel();
    //QWidget* createPlot(int xPos, int yPos, int width, int height);
    QWidget* createPlotByDate(int plotIndex, QRect geometry);
    /// configuration panel
    void createConfigurationPanel();
    void createLabelLine(QGridLayout* l);
    QWidget* createSpacedWidget(QWidget *w, int spaceBefore, int spaceAfter);
    struct PointOnMap {
        QPoint p;
        QGraphicsEllipseItem* circle;
        QGraphicsLineItem* line;
    };
    QList<PointOnMap> wayPoints;
    QList<QGraphicsEllipseItem*> gpsPoints;
    void removeLastWaypoint();
    void sendWaypointCommand(quint8 command, QList<QPointF> points);
    void applyStyle();
    /// export panel
    void createExportPanel();
    void displayDataForMission(QString missionName);
    void updateMissionList();
    /// NS panel
    void updateWPFileList();


    SensorInputsModel *m_sensorInputsModel;
    RegisteredSensorsModel *m_registeredSensorsModel;

    void setEngineControlSlidersConnection(bool enableConnections);
    QGraphicsPolygonItem* m_catPolygon;

    void loadProfile(QDataStream &ds);

};

#endif // MAINWINDOW_H
