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
#include "communication/httpRequester.h"


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
    void on_navSysStart();
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
    void setSliderIsMoving(bool);

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
    void on_addressValueChanged(QString, int);
    void on_nameValueChanged(QString, int);
    void on_typeValueChanged(QString typeName, int);
    void on_displayValueChanged(int, int);
    void on_recordValueChanged(bool, int);
    void on_streamValueChanged(bool, int);
    void on_filenameValueChanged(QString, int);
    void on_saveConfigClicked();
    void on_graphNbValueChanged(int);

    /// sensor transformation and config tables events
    void on_availableSensorsValueChanged();
    void on_addSensorFlClicked();
    void on_registeredSensorButtonClick(QModelIndex&);
    void on_sensorConfigChanged();

    /// Honk and Light
    void on_honkButtonPressed();
    void on_honkButtonReleased();
    void on_lightCheckBoxChange();

    /// Navigation System config
    void on_updateNSBtnClick();
    void on_nsValueChange();
    void on_getNSCongifBtnClick();
    void on_defaultNSConfigClick();

protected slots:
    void on_engineValueAutoUpdate();

    /// export
    void on_exportBtnClicked();
    void on_missionSelectedChanged(QItemSelection);
    void on_backendAddressValueChanged(QString);
    void on_httpRequestDone(QNetworkReply*);

protected:
    void keyPressEvent(QKeyEvent* event);

private:
    Ui::MainWindow *ui;
    Server *server;
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
    /// plots panel
    void clearPlotsPanel();
    void createPlotsPanel();
    //QWidget* createPlot(int xPos, int yPos, int width, int height);
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
    QList<QGraphicsEllipseItem*> gpsPoints;
    void removeLastWaypoint();
    void sendWaypointCommand(quint8 command, QList<QPointF> points);
    void applyStyle();
    /// export panel
    void createExportPanel();
    void displayDataForMission(QString missionName);

    RegisteredSensorsModel *m_registeredSensorsModel;

    void setEngineControlSlidersConnection(bool enableConnections);

};

#endif // MAINWINDOW_H
