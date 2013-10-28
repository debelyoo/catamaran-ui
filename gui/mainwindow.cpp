#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qmath.h>
#include <QGraphicsPixmapItem>
#include <QGroupBox>
#include <QLineEdit>
#include <QComboBox>
#include <QScrollBar>
#include "inRowLineEdit.h"
#include "inRowComboBox.h"
#include "inRowCheckBox.h"

#include "transformation/transformationmanager.h"
#include "gui/sensortransformationconfig.h"
#include "util/criodefinitions.h"
#include "communication/criocommand.h"
#include "communication/criodata.h"
#include "manager/sensortypemanager.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_catPolygon()
{
    zoomStep = 10;
    sensorConfig = SensorConfig::instance();
    fileHelper = FileHelper::instance();
    //converter = ByteArrayConverter::instance();
    coordinateHelper = CoordinateHelper::instance();
    compactRio = CompactRio::instance();
    dataExporter = DataExporter::instance();
    dbManager = DatabaseManager::instance();
    ui->setupUi(this);

    // load map in graphics view (for GPS points)
    QGraphicsScene *scene = new QGraphicsScene();
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(QPixmap(":/images/Map.jpg"));
    item->setPos(0, 0);
    scene->addItem(item);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->viewport()->setObjectName("mapViewport");
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->navModeComboBox->addItem("Manual");
    ui->navModeComboBox->addItem("Automatic");
    ui->waypointGroupBox->hide(); // hide WP panel because mode is manual by default

    createConfigurationPanel();
    createPlotsPanel(); // need to be after configuration panel for plots
    createExportPanel();

    // Server  : signals wiring
    server = Server::instance();
    connect(server, SIGNAL(displayInGui(QString)), this, SLOT(addStatusText(QString)));
    connect(server, SIGNAL(newConnection()), this, SLOT(on_newConnection()));

    connect(compactRio, SIGNAL(newCRioStatusMessage(QString)), this, SLOT(addCrioStatusText(QString)));
    connect(compactRio, SIGNAL(positionChanged()), this, SLOT(on_crioPositionChanged()));
    connect(compactRio, SIGNAL(speedChanged()), this, SLOT(on_crioSpeedChanged()));
    connect(compactRio, SIGNAL(headingChanged()), this, SLOT(on_crioHeadingChanged()));

    sliderIsMoving = false;
    previousSpeedValue = 0;
    previousDirectionValue = 0;

    connect(server, SIGNAL(gpsPointReceived(double, double)), this, SLOT(drawPointOnMap(double,double)));
    connect(ui->navModeComboBox, SIGNAL(activated(int)), this, SLOT(on_navModeChanged(int)));
    // engines' sliders & spinboxes
    connect(ui->leftSlider, SIGNAL(valueChanged(int)), ui->leftSpinBox, SLOT(setValue(int)));
    connect(ui->rightSlider, SIGNAL(valueChanged(int)), ui->rightSpinBox, SLOT(setValue(int)));
    connect(ui->leftSpinBox, SIGNAL(valueChanged(int)), ui->leftSlider, SLOT(setValue(int)));
    connect(ui->rightSpinBox, SIGNAL(valueChanged(int)), ui->rightSlider, SLOT(setValue(int)));

    // speed and direction sliders & spinboxes
    /*
    connect(ui->speedSlider, SIGNAL(valueChanged(int)), this, SLOT(on_speedValueChanged(int)));
    connect(ui->directionSlider, SIGNAL(valueChanged(int)), this, SLOT(on_directionValueChanged(int)));
    connect(ui->speedSpinBox, SIGNAL(valueChanged(int)), this, SLOT(on_speedValueChanged(int)));
    connect(ui->directionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(on_directionValueChanged(int)));
    */
    setEngineControlSlidersConnection(true);
    ui->speedSpinBox->installEventFilter(new MouseClickHandler(this));
    ui->directionSpinBox->installEventFilter(new MouseClickHandler(this));

    connect(ui->speedSlider, SIGNAL(sliderPressed()), this, SLOT(on_sliderPressed()));
    connect(ui->directionSlider, SIGNAL(sliderPressed()), this, SLOT(on_sliderPressed()));
    connect(ui->speedSlider, SIGNAL(sliderReleased()), this, SLOT(on_sliderReleased()));
    connect(ui->directionSlider, SIGNAL(sliderReleased()), this, SLOT(on_sliderReleased()));

    // map view
    connect(ui->zoomInBtn, SIGNAL(clicked()), this, SLOT(zoomIn()));
    connect(ui->zoomOutBtn, SIGNAL(clicked()), this, SLOT(zoomOut()));
    connect(ui->zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));
    ui->graphicsView->viewport()->installEventFilter(new MouseClickHandler(this));
    connect(ui->removeWpBtn, SIGNAL(clicked()), this, SLOT(on_removeWpClicked()));
    connect(ui->clearWpBtn, SIGNAL(clicked()), this, SLOT(on_clearWpClicked()));
    connect(ui->startNavigationWpBtn, SIGNAL(clicked()), this, SLOT(on_navSysStart()));
    connect(ui->cleanGPSBtn, SIGNAL(clicked()), this, SLOT(on_cleanGPSClicked()));

    // Honk and Light
    connect(ui->lightBtn, SIGNAL(clicked()), this, SLOT(on_lightCheckBoxChange()));
    connect(ui->honkBtn, SIGNAL(pressed()), this, SLOT(on_honkButtonPressed()));
    connect(ui->honkBtn, SIGNAL(released()), this, SLOT(on_honkButtonReleased()));

    // Navigation System config
    connect(ui->updateNSBtn, SIGNAL(clicked()), this, SLOT(on_updateNSBtnClick()));
    connect(ui->defaultNSConfigBtn, SIGNAL(clicked()), this, SLOT(on_defaultNSConfigClick()));
    connect(ui->c0, SIGNAL(valueChanged(double)), this, SLOT(on_nsValueChange()));
    connect(ui->c1, SIGNAL(valueChanged(double)), this, SLOT(on_nsValueChange()));
    connect(ui->c2, SIGNAL(valueChanged(double)), this, SLOT(on_nsValueChange()));
    connect(ui->c3, SIGNAL(valueChanged(double)), this, SLOT(on_nsValueChange()));
    connect(ui->c4, SIGNAL(valueChanged(double)), this, SLOT(on_nsValueChange()));
    connect(ui->l0, SIGNAL(valueChanged(double)), this, SLOT(on_nsValueChange()));
    connect(ui->l1, SIGNAL(valueChanged(double)), this, SLOT(on_nsValueChange()));

    // Sensor transformation view
    SensorInputsModel *sensorsInputModel = new SensorInputsModel(this);
    m_registeredSensorsModel = new RegisteredSensorsModel(this);
    ui->registeredSensors->sortByColumn(0);
    ui->availableSensorsInput->setModel(sensorsInputModel);

    RegisteredSensorsModel::Proxy* proxyModel = new RegisteredSensorsModel::Proxy();
    proxyModel->setSourceModel(m_registeredSensorsModel);
    ui->registeredSensors->setModel(proxyModel);

    //ui->registeredSensors->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->registeredSensors->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->registeredSensors->horizontalHeader()->resizeSection(RegisteredSensorsModel::ConfigCol, 120);
    ui->registeredSensors->horizontalHeader()->resizeSection(RegisteredSensorsModel::StreamCol, 80);
    ui->registeredSensors->horizontalHeader()->resizeSection(RegisteredSensorsModel::RecordCol, 80);
    ui->registeredSensors->horizontalHeader()->setSectionResizeMode(RegisteredSensorsModel::AddressCol, QHeaderView::Stretch);
    ui->registeredSensors->horizontalHeader()->setSectionResizeMode(RegisteredSensorsModel::NameEditCol, QHeaderView::Stretch);
    ui->registeredSensors->horizontalHeader()->setSectionResizeMode(RegisteredSensorsModel::TransfCol, QHeaderView::Stretch);

    ui->registeredSensors->setAlternatingRowColors(true);
    ui->registeredSensors->setStyleSheet(/*"QTableView::item:hover {background-color: rgb(185, 210, 235);}*/"QTableView{alternate-background-color: #f3f3f3; background-color: #ffffff;}");

    RegisteredSensorsDelegate * delegate = new RegisteredSensorsDelegate(RegisteredSensorsModel::ConfigCol, ui->registeredSensors);
    ComboBoxDelegate *comboDelegate = new ComboBoxDelegate(RegisteredSensorsModel::TypeCol, ui->registeredSensors);
    connect(delegate, SIGNAL(buttonClicked(QModelIndex&)), this, SLOT(on_registeredSensorButtonClick(QModelIndex&)));
    ui->registeredSensors->setItemDelegateForColumn(RegisteredSensorsModel::ConfigCol, delegate);
    ui->registeredSensors->setItemDelegateForColumn(RegisteredSensorsModel::TypeCol, comboDelegate);
    connect(sensorsInputModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(on_availableSensorsValueChanged()));

    foreach(CompactRio::NamedAddress as, CompactRio::instance()->availableInputs()){
        SensorInputItem * sensor = new SensorInputItem(QString::number(as.address), as.name);
        sensorsInputModel->addInput(sensor);
    }

    connect(ui->addSensorFromList, SIGNAL(clicked()), this, SLOT(on_addSensorFlClicked()));
    connect(m_registeredSensorsModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(on_sensorConfigChanged()));

    applyStyle();

    // handle signal of tab change
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(on_tabChanged(int)));


    TransformationManager::instance()->load();

    //Server : start
    server->listen();

    QPolygon catShape;
    catShape.append(QPoint(0, -2));
    catShape.append(QPoint(1, 1));
    catShape.append(QPoint(-1, 1));
    m_catPolygon = new QGraphicsPolygonItem(catShape);
    m_catPolygon->setPen(QPen(QColor(0, 255, 255)));
    m_catPolygon->setBrush(QBrush(Qt::SolidPattern));
    ui->graphicsView->scene()->addItem(m_catPolygon);
    m_catPolygon->setZValue(999999999);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case 43:
        // '+' key
        if (ui->tabWidget->currentIndex() == 0)
            ui->speedSlider->setValue(ui->speedSlider->value() + 1);
        break;
    case 45:
        // '-' key
        if (ui->tabWidget->currentIndex() == 0)
            ui->speedSlider->setValue(ui->speedSlider->value() - 1);
        break;
    default:
        break;
    }
}

void MainWindow::addStatusText(QString msg)
{
    ui->statusText->insertPlainText(msg);
    QTextCursor c =  ui->statusText->textCursor();
    c.movePosition(QTextCursor::End);
    ui->statusText->setTextCursor(c);
}

void MainWindow::addCrioStatusText(QString msg)
{
    ui->crioStatusText->insertPlainText(msg);
    QTextCursor c =  ui->crioStatusText->textCursor();
    c.movePosition(QTextCursor::End);
    ui->crioStatusText->setTextCursor(c);
}

void MainWindow::updateLeftSpinBox()
{
    ui->leftSpinBox->setValue(ui->leftSlider->value());
}

void MainWindow::updateRightSpinBox()
{
    ui->rightSpinBox->setValue(ui->rightSlider->value());
}

void MainWindow::updateSpeedSpinBox()
{
    ui->speedSpinBox->setValue(ui->speedSlider->value());
}

void MainWindow::updateDirectionSpinBox()
{
    ui->directionSpinBox->setValue(ui->directionSlider->value());
}

void MainWindow::on_sliderPressed()
{
    sliderIsMoving = true;
}

void MainWindow::on_sliderReleased()
{
    sliderIsMoving = false;
    sendEngineCommand();
}

void MainWindow::on_navModeChanged(int modeId)
{
    if (modeId == 0)
    {
        // manual mode
        ui->joystickCheckbox->setEnabled(true);
        ui->speedSlider->setEnabled(true);
        ui->speedSpinBox->setEnabled(true);
        ui->directionSlider->setEnabled(true);
        ui->directionSpinBox->setEnabled(true);
        ui->waypointGroupBox->hide();
        ui->stopBtn->show();
        compactRio->stop();
        compactRio->setNavSysMode(CRIO::NAV_SYS_MANUAL);
        ui->startNavigationWpBtn->setText("Start");
        ui->startNavigationWpBtn->setProperty("started", false);
        disconnect(compactRio, SIGNAL(enginesChanged()), this, 0);
    }
    else
    {
        // automatic mode
        ui->joystickCheckbox->setEnabled(false);
        ui->speedSlider->setEnabled(false);
        ui->speedSpinBox->setEnabled(false);
        ui->directionSlider->setEnabled(false);
        ui->directionSpinBox->setEnabled(false);
        ui->stopBtn->hide();
        ui->waypointGroupBox->show();
        connect(compactRio, SIGNAL(enginesChanged()), this, SLOT(on_engineValueAutoUpdate()));
    }

}

void MainWindow::on_speedValueChanged(int val)
{
    ui->speedSlider->setValue(val);
    ui->speedSpinBox->setValue(val);
    updateLeftRightSliders();
    if (previousSpeedValue != val)
    {
        sendEngineCommand();
        previousSpeedValue = val;
    }
}

void MainWindow::on_directionValueChanged(int val)
{
    ui->directionSlider->setValue(val);
    ui->directionSpinBox->setValue(val);
    updateLeftRightSliders();
    if (previousDirectionValue != val)
    {
        sendEngineCommand();
        previousDirectionValue = val;
    }
}

void MainWindow::on_saveConfigClicked()
{
    // write config file
    fileHelper->writeFile("config2.txt", sensorConfig->getSensorsAsTabSeparatedText(), false);
    fileHelper->createLogFiles();
    addStatusText("Config saved !\n");
    changeSaveBtnColor("gray");
    compactRio->setSensorsConfig();
    clearPlotsPanel();
    createPlotsPanel();
}

void MainWindow::on_addressValueChanged(QString addrStr, int rowIndex)
{
    /*
    Sensor* s = sensorConfig->getSensors()[rowIndex];
    bool b;
    int addr = addrStr.toInt(&b);
    if (b)
    {
        s->setAddress(addrStr);
        QString msg = "Address ["+ QString::number(rowIndex) +"] changed: " + addrStr + "\n";
        addStatusText(msg);
    }
    else
    {
        QString msg = "Address: " + addrStr + " - Bad format !\n";
        addStatusText(msg);
    }
    */
}

void MainWindow::on_nameValueChanged(QString name, int rowIndex)
{
    Sensor* s = sensorConfig->getSensors()[rowIndex];
    s->setName(name);
    QString msg = "Name ["+ QString::number(rowIndex) +"] changed: " + name + "\n";
    addStatusText(msg);
    changeSaveBtnColor("red");
}

void MainWindow::on_typeValueChanged(QString typeName, int rowIndex)
{
    const SensorType *type = SensorTypeManager::instance()->type(typeName);
    Sensor* s = sensorConfig->getSensors()[rowIndex];
    s->setType(type);
    QString msg = "Type ["+ QString::number(rowIndex) +"] changed: " + type->getName() + "\n";
    addStatusText(msg);
    changeSaveBtnColor("red");
}

void MainWindow::on_displayValueChanged(int displayInd, int rowIndex)
{
    Sensor* s = sensorConfig->getSensors()[rowIndex];
    s->setDisplay(displayInd);
    QString msg = "Display ["+ QString::number(rowIndex) +"] changed: " + sensorConfig->getDisplayValues().value(displayInd) + "\n";
    addStatusText(msg);
    changeSaveBtnColor("red");
}

void MainWindow::on_recordValueChanged(bool rec, int rowIndex)
{
    Sensor* s = sensorConfig->getSensors()[rowIndex];
    s->setRecord(rec);
    QString msg = "Record ["+ QString::number(rowIndex) +"] changed: " + QString::number(rec) + "\n";
    addStatusText(msg);
    changeSaveBtnColor("red");
}

void MainWindow::on_streamValueChanged(bool stream, int rowIndex)
{
    Sensor* s = sensorConfig->getSensors()[rowIndex];
    s->setStream(stream);
    QString msg = "Stream ["+ QString::number(rowIndex) +"] changed: " + QString::number(stream) + "\n";
    addStatusText(msg);
    changeSaveBtnColor("red");
}

void MainWindow::on_filenameValueChanged(QString fn, int rowIndex)
{
    QString msg = "Filename ["+ QString::number(rowIndex) +"] changed: " + fn + "\n";
    addStatusText(msg);
    changeSaveBtnColor("red");
}

/**
 * SLOT called when "nb of graphs" value has changed. Update this value in config and redraw addresses config
 * @brief MainWindow::on_graphNbValueChanged
 * @param nb The new value for graph number
 */
void MainWindow::on_graphNbValueChanged(int nb)
{
    sensorConfig->updateDisplayGraphList(nb);
    clearAddressesConfigPanel();
    createAddressesConfigPanel();
}

void MainWindow::on_availableSensorsValueChanged()
{
    ui->availableSensorsInput->expandAll();
}

void MainWindow::on_addSensorFlClicked()
{
    QModelIndexList indexes = ui->availableSensorsInput->selectionModel()->selectedRows();
    for(int i=0;i<indexes.count(); ++i){
        SensorInputItem *sensor = static_cast<SensorInputItem *>(indexes[i].internalPointer());
        if(sensor && sensor->enabled()){
            sensor->disable();
            m_registeredSensorsModel->addSensor(new RegisteredSensorItem(sensor));
            ui->registeredSensors->sortByColumn(0);
        }
    }
}

void MainWindow::on_registeredSensorButtonClick(QModelIndex &index)
{
    QSortFilterProxyModel * proxyModel = static_cast<QSortFilterProxyModel *>(ui->registeredSensors->model());
    QModelIndex realIndex = proxyModel->mapToSource(index);
    SensorTransformationConfig *widget = new SensorTransformationConfig(static_cast<RegisteredSensorItem *>(realIndex.internalPointer()), this);
    widget->show();
}

void MainWindow::on_sensorConfigChanged()
{
    compactRio->setSensorsConfig();
}

void MainWindow::on_honkButtonPressed()
{
    compactRio->setHonk(CRIO::ON);
}

void MainWindow::on_honkButtonReleased()
{
    compactRio->setHonk(CRIO::OFF);
}

void MainWindow::on_lightCheckBoxChange()
{
    compactRio->setLight(ui->lightBtn->isChecked()?CRIO::ON:CRIO::OFF);
}

void MainWindow::on_updateNSBtnClick()
{
    compactRio->setNavSysConstants(ui->c0->value(), ui->c1->value(), ui->c2->value(), ui->c3->value(), ui->c4->value());
    compactRio->setNavSysLimits(ui->l0->value(), ui->l1->value());

    ui->updateNSBtn->setStyleSheet("QPushButton {" \
                                   "    border: 2px solid #72A574;" \
                                   "    border-radius: 4px;" \
                                   "    border-style: ridge;" \
                                   "    background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #C5F9C8, stop: 1 #AEE2B2);" \
                                   "}" \
                                   "QPushButton:pressed {" \
                                   "    background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #AEE2B2, stop: 1 #C5F9C8);" \
                                   "}");
}

void MainWindow::on_nsValueChange()
{
    ui->updateNSBtn->setStyleSheet("QPushButton {" \
                                   "    border: 2px solid #A57274;" \
                                   "    border-radius: 4px;" \
                                   "    border-style: ridge;" \
                                   "    background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #F8C6C8, stop: 1 #E1AFB2);" \
                                   "}" \
                                   "QPushButton:pressed {" \
                                   "    background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E1AFB2, stop: 1 #F8C6C8);" \
                                   "}");
}

void MainWindow::on_getNSCongifBtnClick()
{
    compactRio->getCommand(CRIO::CMD_ADDR_NS_CSTS);
    compactRio->getCommand(CRIO::CMD_ADDR_NS_LIMITS);
}

void MainWindow::on_defaultNSConfigClick()
{
    ui->c0->setValue(4.89796);
    ui->c1->setValue(9.79592);
    ui->c2->setValue(66.3265);
    ui->c3->setValue(21.9388);
    ui->c4->setValue(11.2245);
    ui->l0->setValue(25);
    ui->l1->setValue(0.5);
}

void MainWindow::on_crioPositionChanged()
{
    QPointF p = compactRio->position();
    ui->positionLabel->setText(QString("%1, %2 [°]").arg(p.x(), 7).arg(p.y(), 7));
    QVector<double> v = coordinateHelper->WGS84toLV03(p.x(),p.y(),273);
    m_catPolygon->setPos(coordinateHelper->LV03toUIMap(v[0], v[1]));
}

void MainWindow::on_crioSpeedChanged()
{
    QPointF s = compactRio->speed();
    QPointF ms = compactRio->meanSpeed();
    ui->speedLabel->setText(QString("%1(%3), %2(%4) [m/s]").arg(s.x(), 3, ' ', 1).arg(s.y(), 3, ' ', 1).arg(ms.x(), 3, ' ', 1).arg(ms.y(), 3, ' ', 1));
}

void MainWindow::on_crioHeadingChanged()
{
    ui->headingLabel->setText(QString("%1 [°]").arg(compactRio->heading(), 6, ' ', 2));
    m_catPolygon->setRotation(compactRio->heading());
    m_catPolygon->update();
    //QPoint p = ui->graphicsView->sceneRect()
    //ui->graphicsView->update();
}

void MainWindow::on_engineValueAutoUpdate()
{
    setEngineControlSlidersConnection(false);
    int left = compactRio->leftEngineValue()/1.27;
    int right = compactRio->rightEngineValue()/1.27;
    ui->leftSlider->setValue(left);
    ui->rightSlider->setValue(right);
    updateSpeedDirectionSliders(left, right);
    qDebug() << "on_engineValueAutoUpdate called: left="<<left<<" right="<<right;
    setEngineControlSlidersConnection(true);
}

void MainWindow::zoomIn()
{
    ui->zoomSlider->setValue(ui->zoomSlider->value() + zoomStep);
}

void MainWindow::zoomOut()
{
    ui->zoomSlider->setValue(ui->zoomSlider->value() - zoomStep);
}

void MainWindow::setupMatrix()
{
    qreal scale = qPow(qreal(2), (ui->zoomSlider->value() - 50) / qreal(10));

    QMatrix matrix;
    matrix.scale(scale, scale);

    ui->graphicsView->setMatrix(matrix);
}

void MainWindow::on_removeWpClicked()
{
    removeLastWaypoint();
}

void MainWindow::on_clearWpClicked()
{
    while ( !wayPoints.isEmpty() )
    {
        removeLastWaypoint();
    }
}

/**
 * Remove all GPS points on map
 * @brief MainWindow::on_cleanGPSClicked
 */
void MainWindow::on_cleanGPSClicked()
{
    while (!gpsPoints.isEmpty())
    {
        delete gpsPoints.first();
    }
}

void MainWindow::on_exportBtnClicked()
{
    // export data
    QString missionName = ui->listViewMission->selectionModel()->selectedIndexes().first().data().toString();
    QString datatype = ui->listViewData->selectionModel()->selectedIndexes().first().data().toString();
    //qDebug() << "on_exportBtnClicked() - " << missionName << ", " << datatype;
    dataExporter->exportData(missionName, datatype);
}

void MainWindow::on_removeMissionBtnClicked()
{
    QString missionName = ui->listViewMission->selectionModel()->selectedIndexes().first().data().toString();
    if (missionName == dbManager->getCurrentMissionName()) {
        addStatusText("[Warning] Current mission can not be deleted ! \n");
        return;
    }
    dbManager->removeMission(missionName);
    //qDebug() << "on_removeMissionBtnClicked() - " << missionName << " - "<< b;
    updateMissionList();
}

/**
 * This method is called when the selected mission changes
 * @brief MainWindow::on_missionSelectedChanged
 * @param selection
 */
void MainWindow::on_missionSelectedChanged(QItemSelection selection)
{
    // load data for mission
    QModelIndex ind = (QModelIndex)selection.indexes().first();
    //qDebug() << "on_missionSelectedChanged()" << ind.row() << " - " << ind.data().toString() ;
    displayDataForMission(ind.data().toString());
}

void MainWindow::on_backendAddressValueChanged(QString addr)
{
    HttpRequester::instance()->setBackendAddress(addr);
}

/**
 * This method is called when a ping request is done
 * @brief MainWindow::on_pingRequestDone
 * @param statusCode The HTTP status code
 */
void MainWindow::on_pingRequestDone(int statusCode)
{
    if (statusCode == 200) {
        ui->backendStatusBtn->setText("Online");
        ui->backendStatusBtn->setStyleSheet("QPushButton {" \
                                       "    border: 2px solid #72A574;" \
                                       "    border-radius: 4px;" \
                                       "    border-style: ridge;" \
                                       "    padding: 5px;" \
                                       "    background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #C5F9C8, stop: 1 #AEE2B2);" \
                                       "}" \
                                       "QPushButton:pressed {" \
                                       "    background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #AEE2B2, stop: 1 #C5F9C8);" \
                                       "}");
    } else {
        ui->backendStatusBtn->setText("Offline");
        ui->backendStatusBtn->setStyleSheet("QPushButton {" \
                                       "    border: 2px solid #A57274;" \
                                       "    border-radius: 4px;" \
                                       "    border-style: ridge;" \
                                       "    padding: 5px;" \
                                       "    background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #F8C6C8, stop: 1 #E1AFB2);" \
                                       "}" \
                                       "QPushButton:pressed {" \
                                       "    background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E1AFB2, stop: 1 #F8C6C8);" \
                                       "}");
    }
}

/**
 * Scroll the plots container when scroll is triggered over a plot
 * @brief MainWindow::on_graphWheelEvent
 * @param ev The wheel event triggered on plot
 */
void MainWindow::on_graphWheelEvent(QWheelEvent* ev)
{
    QList<QWidget*> list = ui->tabWidget->findChildren<QWidget*>("scrollAreaPlotsPanel");
    QScrollArea* sa = (QScrollArea*)list.first();
    sa->verticalScrollBar()->setValue(sa->verticalScrollBar()->value() - ev->delta());
}

void MainWindow::on_newConnection()
{
    compactRio->setFpgaCounterSamplingTime(2250);
    compactRio->setSabertoothState(CRIO::ON);
}

/**
 * This SLOT is called when selected tab changes
 * @brief MainWindow::on_tabChanged
 * @param tabIndex The index of the selected tab
 */
void MainWindow::on_tabChanged(int tabIndex)
{
    if (ui->tabWidget->widget(tabIndex)->objectName() == "export_tab") {
        updateMissionList();
    }
}

void MainWindow::setSliderIsMoving(bool b)
{
    sliderIsMoving = b;
}

/**
 * Draw a GPS point on map, center map on this point
 * @brief MainWindow::drawPointOnMap
 * @param x The x coordinate of the point
 * @param y The y coordinate of the point
 */
void MainWindow::drawPointOnMap(double x, double y)
{
    double rad = 1;
    QColor color = QColor(255, 90, 0); // orange
    QGraphicsEllipseItem* pt = ui->graphicsView->scene()->addEllipse(x-rad, y-rad, rad, rad, QPen(color), QBrush(Qt::SolidPattern));
    gpsPoints.push_back(pt); // add the point in array to be remove when click on "clean GPS points" button
    // center view on new point
    ui->graphicsView->centerOn(x, y);
}

/**
 * Draw a way point on map
 * @brief MainWindow::drawWayPointOnMap
 * @param newPoint The new way point to dram on map
 */
void MainWindow::drawWayPointOnMap(QPoint newPoint)
{
    if (ui->navModeComboBox->currentIndex() == 1)
    {
        QPoint previousPoint;
        if (!wayPoints.empty())
            previousPoint = ((PointOnMap)wayPoints.last()).p;
        QPointF pointFloat = ui->graphicsView->mapToScene(newPoint);
        QPoint pointOnMap = pointFloat.toPoint();
        PointOnMap pom;
        pom.p = pointOnMap;
        double rad = 1;
        QColor color = QColor(124, 252, 0); // green
        pom.circle = ui->graphicsView->scene()->addEllipse(pointOnMap.x()-rad, pointOnMap.y()-rad, rad, rad, QPen(color), QBrush(Qt::SolidPattern));
        if (!previousPoint.isNull())
            pom.line = ui->graphicsView->scene()->addLine(previousPoint.x(), previousPoint.y(), pointOnMap.x(), pointOnMap.y(), QPen(color));
        else
            pom.line = 0;
        wayPoints.push_back(pom);
        QList<QPointF> ptList;
        ptList.push_back(coordinateHelper->UIMaptoLV03(pointFloat));
        // send command to cRIO
        sendWaypointCommand(MessageUtil::Add, ptList);
    }
}

void MainWindow::on_navSysStart()
{
    QVariant started = ui->startNavigationWpBtn->property("started");
    if(!started.isValid() || !started.toBool()){
        compactRio->setNavSysMode(CRIO::NAV_SYS_AUTO);
        ui->startNavigationWpBtn->setText("Stop");
        ui->startNavigationWpBtn->setProperty("started", true);
    }else{
        compactRio->setNavSysMode(CRIO::NAV_SYS_MANUAL);
        compactRio->stop();
        ui->startNavigationWpBtn->setText("Start");
        ui->startNavigationWpBtn->setProperty("started", false);
    }
}

/**
 * Send a waypoint command (Set/Add/Delete) to cRIO
 * @brief MainWindow::sendWaypointCommand
 * @param command The command id (Set/Add/Delete)
 * @param points A list of points to send (empty with Delete command)
 */
void MainWindow::sendWaypointCommand(quint8 command, QList<QPointF> points)
{
    switch (command) {
    case MessageUtil::Set:
        compactRio->setWaypointsCmd(points);
        break;
    case MessageUtil::Add:
        compactRio->addWaypointCmd(points[0]);
        break;
    case MessageUtil::Delete:
        compactRio->delWaypointCmd();
        break;
    default:
        break;
    }
    //qDebug() << "sendWaypointCommand()";
}

void MainWindow::applyStyle()
{
    ui->availableSensorsInput->setStyleSheet(
        "QTreeView::branch:has-siblings:!adjoins-item {" \
        "   border-image: url(:/images/ressources/style/stylesheet-vline.png) 0;" \
        "}" \
        "QTreeView::branch:has-siblings:adjoins-item {" \
        "    border-image: url(:/images/ressources/style/stylesheet-branch-more.png) 0;" \
        "}" \
        "QTreeView::branch:!has-children:!has-siblings:adjoins-item {" \
        "    border-image: url(:/images/ressources/style/stylesheet-branch-end.png) 0;" \
        "}" \
        "QTreeView::branch:has-children:!has-siblings:closed," \
        "QTreeView::branch:closed:has-children:has-siblings {" \
        "        border-image: none;" \
        "        image: url(:/images/ressources/style/stylesheet-branch-closed.png);" \
        "}" \
        "QTreeView::branch:open:has-children:!has-siblings," \
        "QTreeView::branch:open:has-children:has-siblings  {" \
        "        border-image: none;" \
        "        image: url(:/images/ressources/style/stylesheet-branch-open.png);" \
        "}"
                );
}

void MainWindow::setEngineControlSlidersConnection(bool enableConnections)
{
    if(enableConnections){
        connect(ui->speedSlider, SIGNAL(valueChanged(int)), this, SLOT(on_speedValueChanged(int)));
        connect(ui->directionSlider, SIGNAL(valueChanged(int)), this, SLOT(on_directionValueChanged(int)));
        connect(ui->speedSpinBox, SIGNAL(valueChanged(int)), this, SLOT(on_speedValueChanged(int)));
        connect(ui->directionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(on_directionValueChanged(int)));
    } else {
        disconnect(ui->speedSlider, SIGNAL(valueChanged(int)), this, SLOT(on_speedValueChanged(int)));
        disconnect(ui->directionSlider, SIGNAL(valueChanged(int)), this, SLOT(on_directionValueChanged(int)));
        disconnect(ui->speedSpinBox, SIGNAL(valueChanged(int)), this, SLOT(on_speedValueChanged(int)));
        disconnect(ui->directionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(on_directionValueChanged(int)));
    }
}

void MainWindow::sendEngineCommand()
{
    if (!sliderIsMoving)
    {
        //qDebug() << "sendEngineCommand";
        sendLeftEngineCommand();
        sendRightEngineCommand();
    }
}

/// private methods
void MainWindow::sendLeftEngineCommand()
{
    // command (uint8) | length array (uint32) | length engine addr (uint32) |engine addr (uint8) | length value (uint32) | value (int8)
    int val = correctEngineCommandValue(ui->leftSlider->value());
    compactRio->setEngine(CRIO::LEFT, (qint8) val);
    qDebug() << "sendLeftEngineCommand() [" << val << "]";
}

void MainWindow::sendRightEngineCommand()
{
    int val = correctEngineCommandValue(ui->rightSlider->value());
    compactRio->setEngine(CRIO::RIGHT, (qint8) val);
    qDebug() << "sendRightEngineCommand() [" << val << "]";
}

/**
 * Correct the value for the engine command. In UI range is from -100 to 100, but engines take values between -127 to 127.
 * @brief MainWindow::correctEngineCommandValue
 * @param val
 * @return
 */
int MainWindow::correctEngineCommandValue(int val)
{
    int v2 = val*1.27;
    return v2>127?127:(v2<-127?-127:v2);
}

/**
 * Update the left and right sliders according to speed and direction values
 * @brief MainWindow::updateLeftRightSliders
 */
void MainWindow::updateLeftRightSliders()
{
    int right = ui->speedSlider->value() - ui->directionSlider->value();
    int left = ui->speedSlider->value() + ui->directionSlider->value();
    ui->rightSlider->setValue(right);
    ui->leftSlider->setValue(left);
}

void MainWindow::updateSpeedDirectionSliders(int left, int right)
{
    ui->speedSlider->setValue((left+right)/2);
    ui->directionSlider->setValue((left-right)/2);
}

/**
 * Create the line with the column titles
 * @brief MainWindow::createLabelLine
 * @param layout
 */
void MainWindow::createLabelLine(QGridLayout* layout)
{
    QLabel *l1 = new QLabel("Address");
    QLabel *l2 = new QLabel("Name");
    QLabel *l3 = new QLabel("Type");
    QLabel *l4 = new QLabel("Display");
    QLabel *l5 = new QLabel("Record");
    QLabel *l6 = new QLabel("Stream");
    QLabel *l7 = new QLabel("Filename");
    layout->addWidget(l1,0,0);
    layout->addWidget(l2,0,1);
    layout->addWidget(l3,0,2);
    layout->addWidget(l4,0,3);
    layout->addWidget(l5,0,4);
    layout->addWidget(l6,0,5);
    layout->addWidget(l7,0,6);
}

/**
 * Add spacing before and after a widget
 * @brief MainWindow::createSpacedWidget
 * @param w The widget to surround by space
 * @param spaceBefore The size of space left to the w
 * @param spaceAfter The size of space right to w
 * @return A container widget with w surrounded by spaces
 */
QWidget* MainWindow::createSpacedWidget(QWidget *w, int spaceBefore, int spaceAfter)
{
    QWidget *container = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout;
    QSpacerItem *spacerBefore = new QSpacerItem(spaceBefore, 10); // for spacing left to checkbox
    QSpacerItem *spacerAfter = new QSpacerItem(spaceAfter, 10); // for spacing right to checkbox
    container->setLayout(layout);
    if (spaceBefore != 0)
        layout->addSpacerItem(spacerBefore);
    layout->addWidget(w);
    if (spaceAfter != 0)
        layout->addSpacerItem(spacerAfter);
    return container;
}

/**
 * Create the fields on a row for an address (sensor)
 * @brief MainWindow::createAddressFormRow
 * @param layout
 * @param rowIndex
 * @param s The sensor at this address
 */
void MainWindow::createAddressFormRow(QGridLayout* layout, int rowIndex, Sensor* s)
{
    int sensorIndex = rowIndex - 1;
    InRowLineEdit* addressField = new InRowLineEdit(this, sensorIndex);
    addressField->setText(s->address());
    addressField->setFixedWidth(30);
    addressField->setEnabled(false);
    QWidget* addressFieldContainer = createSpacedWidget(addressField, 0, 30);
    InRowLineEdit* nameField = new InRowLineEdit(this, sensorIndex);
    nameField->setText(QString(s->name()));
    InRowComboBox* typeBox = new InRowComboBox(this, sensorIndex);

    foreach(QString s, SensorTypeManager::instance()->list())
    {
        typeBox->addItem(s);
    }
    typeBox->setCurrentIndex(s->type()->getId());
    QWidget* typeBoxContainer = createSpacedWidget(typeBox, 0, 10);
    InRowComboBox* displayBox = new InRowComboBox(this, sensorIndex);
    QMap<int, QString> displayGraphs = sensorConfig->getDisplayValues();
    for (int i=0; i < displayGraphs.size(); i++)
    {
        displayBox->addItem(displayGraphs.value(i));
    }
    displayBox->setCurrentIndex(s->display());
    QWidget* displayBoxContainer = createSpacedWidget(displayBox, 0, 10);
    InRowCheckBox* recordCB = new InRowCheckBox(this, sensorIndex);
    recordCB->setChecked(s->record());
    QWidget* recordCBContainer = createSpacedWidget(recordCB, 10, 0);
    InRowCheckBox* streamCB = new InRowCheckBox(this, sensorIndex);
    streamCB->setChecked(s->stream());
    QWidget* streamCBContainer = createSpacedWidget(streamCB, 10, 0);
    InRowLineEdit* filenameField = new InRowLineEdit(this, sensorIndex);
    filenameField->setText(s->logFilePrefix());
    layout->addWidget(addressFieldContainer, rowIndex, 0);
    layout->addWidget(nameField, rowIndex, 1);
    layout->addWidget(typeBoxContainer, rowIndex, 2);
    layout->addWidget(displayBoxContainer, rowIndex, 3);
    layout->addWidget(recordCBContainer, rowIndex, 4);
    layout->addWidget(streamCBContainer, rowIndex, 5);
    layout->addWidget(filenameField, rowIndex, 6);
    /// handle value changes
    connect(addressField, SIGNAL(textChanged(QString, int)), this, SLOT(on_addressValueChanged(QString, int)));
    connect(nameField, SIGNAL(textChanged(QString, int)), this, SLOT(on_nameValueChanged(QString, int)));
    connect(typeBox, SIGNAL(valueChanged(QString,int)), this, SLOT(on_typeValueChanged(QString, int)));
    connect(displayBox, SIGNAL(valueChanged(int,int)), this, SLOT(on_displayValueChanged(int,int)));
    connect(recordCB, SIGNAL(clicked(bool,int)), this, SLOT(on_recordValueChanged(bool,int)));
    connect(streamCB, SIGNAL(clicked(bool,int)), this, SLOT(on_streamValueChanged(bool,int)));
    connect(filenameField, SIGNAL(textChanged(QString, int)), this, SLOT(on_filenameValueChanged(QString, int)));
}

/**
 * Removes all widgets in the plots panel
 * @brief MainWindow::clearPlotsPanel
 */
void MainWindow::clearPlotsPanel()
{
    QWidget *plotsPanel = ui->tabWidget->widget(1);
    if ( plotsPanel->layout() != NULL )
    {
        QLayoutItem* item;
        while ( ( item = plotsPanel->layout()->takeAt( 0 ) ) != NULL )
        {
            delete item->widget();
            delete item;
        }
        delete plotsPanel->layout();
    }
}

/**
 * Create the panel containing the plots (tab 2)
 * @brief MainWindow::createPlotsPanel
 */
void MainWindow::createPlotsPanel()
{
    QWidget *plotsPanel = ui->tabWidget->widget(1);
    QScrollArea *scrollArea = new QScrollArea;
    QWidget *viewport = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    viewport->setLayout(layout);
    scrollArea->setObjectName("scrollAreaPlotsPanel");
    int nbPlots = sensorConfig->getDisplayValues().size();
    ui->graphNbSpinBox->setValue(nbPlots);
    // Need to use fixed sizes, because container is expandable, cannot use ui->tabWidget->width()
    int plotHeight = 200;
    int plotWidth = 1200;
    //viewport->setGeometry(0,0, 800, nbPlots * plotHeight);
    for (int i = 0; i < nbPlots; i++)
    {
        QWidget* plot;
        QRect geom = QRect(0, i * plotHeight, plotWidth, plotHeight);
        plot = createPlotByDate(i, geom);
        layout->addWidget(plot);
    }
    QVBoxLayout *plotsPanelLayout = new QVBoxLayout;
    plotsPanel->setLayout(plotsPanelLayout);
    scrollArea->setWidget(viewport);
    plotsPanelLayout->addWidget(scrollArea);
}
/*
QWidget* MainWindow::createPlot(int xPos, int yPos, int width, int height)
{
    QWidget *viewport = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    viewport->setLayout(layout);

    QCustomPlot* plot1 = new QCustomPlot;
    layout->addWidget(plot1);
    // add title layout element:
    //plot1->plotLayout()->insertRow(0);
    //plot1->plotLayout()->addElement(0, 0, new QCPPlotTitle(plot1, "GraphX"));
    plot1->setGeometry(xPos, yPos, width, height);
    // generate some data:
    QVector<double> x(101), y(101); // initialize with entries 0..100
    for (int i=0; i<101; ++i)
    {
      x[i] = i/50.0 - 1; // x goes from -1 to 1
      y[i] = x[i]*x[i]; // let's plot a quadratic function
    }
    // create graph and assign data to it:
    plot1->addGraph();
    plot1->graph(0)->setData(x, y);
    // give the axes some labels:
    plot1->xAxis->setLabel("x");
    plot1->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    plot1->xAxis->setRange(-1, 1);
    plot1->yAxis->setRange(0, 1);
    plot1->replot();
    return viewport;
}
*/
/**
 * Create a plot with date as x axis
 * @brief MainWindow::createPlotByDate
 * @param plotIndex
 * @param geometry
 * @return
 */
QWidget* MainWindow::createPlotByDate(int plotIndex, QRect geometry)
{
    QWidget *viewport = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    viewport->setLayout(layout);

    QList<Sensor*> sensorsToPlot = sensorConfig->getSensorsForPlot(plotIndex+1); // plotIndex + 1 because index 0 is for "NO" plot
    if (sensorsToPlot.length() > 0)
    {
        DataPlot* dataPlot = new DataPlot(this, sensorsToPlot);
        layout->addWidget(dataPlot);
        dataPlot->setFixedHeight(geometry.height());
        dataPlot->setMinimumWidth(geometry.width());
        dataPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        connect(dataPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(on_graphWheelEvent(QWheelEvent*)));
        QTimer *timer = new QTimer();
        timer->setInterval(3000);
        connect(timer, SIGNAL(timeout()), dataPlot, SLOT(updatePlot()));
        timer->start();
        layout->addWidget(dataPlot);
    }
    return viewport;
}

/**
 * Removes all widgets in the addresses panel
 * @brief MainWindow::clearAddressesConfigPanel
 */
void MainWindow::clearAddressesConfigPanel()
{
    QWidget *addressesPanel = ui->addressesContainer;
    if ( addressesPanel->layout() != NULL )
    {
        QLayoutItem* item;
        while ( ( item = addressesPanel->layout()->takeAt( 0 ) ) != NULL )
        {
            delete item->widget();
            delete item;
        }
        delete addressesPanel->layout();
    }
}

/**
 * Create the configuration panel
 * @brief MainWindow::createConfigurationPanel
 */
void MainWindow::createConfigurationPanel()
{

    ui->pt100_module1_comboBox->addItem("Low res.");
    ui->pt100_module1_comboBox->addItem("High res.");
    ui->pt100_module2_comboBox->addItem("Low res.");
    ui->pt100_module2_comboBox->addItem("High res.");
    connect(ui->graphNbSpinBox, SIGNAL(valueChanged(int)), this, SLOT(on_graphNbValueChanged(int)));

    createAddressesConfigPanel();
}

void MainWindow::createAddressesConfigPanel()
{
    // load sensorTypes file
    fileHelper->loadSensorTypesFile();
    // load config file
    fileHelper->loadConfigFile();
    // create log files
    fileHelper->createLogFiles();
    QList<Sensor*> sensors = sensorConfig->getSensors();

    //QWidget *configurationPanel = ui->tabWidget->widget(2);
    QWidget *configurationPanel = ui->addressesContainer;
    QScrollArea *scrollArea = new QScrollArea;
    QWidget *viewport = new QWidget;
    QGridLayout *layout = new QGridLayout;
    viewport->setLayout(layout);

    createLabelLine(layout);
    for (int i = 1; i <= sensors.length(); i++)
    {
        createAddressFormRow(layout, i, sensors[i-1]);
    }
    scrollArea->setWidget(viewport);

    QVBoxLayout *configurationPanelLayout = new QVBoxLayout;
    configurationPanel->setLayout(configurationPanelLayout);
    QWidget *scrollAreaContainer = createSpacedWidget(scrollArea,0,250);
    configurationPanelLayout->addWidget(scrollAreaContainer);
    saveBtn = new QPushButton("Save config");
    saveBtn->setStyleSheet("background-color: gray;border-style: outset;border-width: 2px;border-radius: 10px;border-color: black;font: 12px;min-width: 10em;padding: 6px;");
    QWidget *saveBtnContainer = createSpacedWidget(saveBtn,200,200);
    configurationPanelLayout->addWidget(saveBtnContainer);
    connect(saveBtn, SIGNAL(clicked()), this, SLOT(on_saveConfigClicked()));
}

/**
 * Change the color of the "save config" button border
 * @brief MainWindow::changeSaveBtnColor
 * @param cssColor The color to set
 */
void MainWindow::changeSaveBtnColor(QString cssColor)
{
    QString style = "background-color: gray;border-style: outset;border-width: 2px;border-radius: 10px;font: 12px;min-width: 10em;padding: 6px;";
    saveBtn->setStyleSheet(style + "border-color: "+cssColor+";");
}

void MainWindow::removeLastWaypoint()
{
    if(wayPoints.empty()){
        return;
    }
    PointOnMap pom = (PointOnMap)wayPoints.last();
    delete pom.circle;
    if (pom.line != 0)
        delete pom.line;
    wayPoints.removeLast();
    // send command to cRIO (with empty point list)
    QList<QPointF> ptList;
    sendWaypointCommand(MessageUtil::Delete, ptList);
}

void MainWindow::createExportPanel()
{
    ui->comboBoxTimezone->addItem("GMT+2");
    ui->comboBoxTimezone->addItem("GMT+9");

    // load missions in list view
    updateMissionList();

    ui->backendStatusBtn->setStyleSheet("QPushButton {" \
                                   "    border: 2px solid #A57274;" \
                                   "    border-radius: 4px;" \
                                   "    border-style: ridge;" \
                                   "    padding: 5px;" \
                                   "    background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #F8C6C8, stop: 1 #E1AFB2);" \
                                   "}" \
                                   "QPushButton:pressed {" \
                                   "    background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E1AFB2, stop: 1 #F8C6C8);" \
                                   "}");

    connect(ui->listViewMission->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(on_missionSelectedChanged(QItemSelection)));
    connect(ui->exportBtn, SIGNAL(clicked()), this, SLOT(on_exportBtnClicked()));
    connect(ui->removeMissionBtn, SIGNAL(clicked()), this, SLOT(on_removeMissionBtnClicked()));
    connect(ui->backendAddressField, SIGNAL(textChanged(QString)), this, SLOT(on_backendAddressValueChanged(QString)));
    connect(dataExporter, SIGNAL(pingRequestDone(int)), this, SLOT(on_pingRequestDone(int)));
    connect(dataExporter, SIGNAL(displayInGui(QString)), this, SLOT(addStatusText(QString)));

    dataExporter->sendPingRequest();
    QTimer *timer = new QTimer();
    timer->setInterval(5000);
    connect(timer, SIGNAL(timeout()), dataExporter, SLOT(sendPingRequest()));
    timer->start();

    HttpRequester::instance()->setBackendAddress(ui->backendAddressField->text());
}

void MainWindow::displayDataForMission(QString missionName)
{
    QStandardItemModel *model = dbManager->getDataForMissionsAsModel(missionName);
    ui->listViewData->setModel(model);
    ui->listViewData->setCurrentIndex(model->index(0,0));
}

/**
 * Update the list of missions (in left list view)
 * @brief MainWindow::updateMissionList
 */
void MainWindow::updateMissionList()
{
    QStandardItemModel *model = dbManager->getMissionsAsModel();
    ui->listViewMission->setModel(model);
    ui->listViewMission->setCurrentIndex(model->index(0,0));
    // load data for mission
    displayDataForMission(model->index(0,0).data().toString());
}
