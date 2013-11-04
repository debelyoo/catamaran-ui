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

#include "util/hierarchicalidentifier.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_sensorInputsModel(NULL),
    m_registeredSensorsModel(NULL),
    m_catPolygon()
{
    zoomStep = 10;
    sensorConfig = SensorConfig::instance();
    fileHelper = FileHelper::instance();
    //converter = ByteArrayConverter::instance();
    coordinateHelper = CoordinateHelper::instance();
    compactRio = CompactRio::instance();
    dataExporter = DataExporter::instance();
    httpRequester = HttpRequester::instance();
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

    ui->graphNbSpinBox->setValue(3);

    connect(ui->actionSave_config, SIGNAL(triggered()), this, SLOT(on_saveConfig()));
    connect(ui->actionLoad_configuration, SIGNAL(triggered()), this, SLOT(on_loadConfig()));

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
    connect(ui->loadWPFileBtn, SIGNAL(clicked()), this, SLOT(on_loadWPBtnClicked()));
    updateWPFileList();

    // Sensor transformation view
    m_sensorInputsModel = new SensorInputsModel(this);
    m_registeredSensorsModel = new RegisteredSensorsModel(this);
    ui->registeredSensors->sortByColumn(0);
    ui->availableSensorsInput->setModel(m_sensorInputsModel);

    RegisteredSensorsModel::Proxy* proxyModel = new RegisteredSensorsModel::Proxy();
    proxyModel->setSourceModel(m_registeredSensorsModel);
    ui->registeredSensors->setModel(proxyModel);

    //ui->registeredSensors->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->registeredSensors->horizontalHeader()->setStretchLastSection(false);
    ui->registeredSensors->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->registeredSensors->horizontalHeader()->resizeSection(RegisteredSensorsModel::ConfigCol, 120);
    ui->registeredSensors->horizontalHeader()->resizeSection(RegisteredSensorsModel::StreamCol, 80);
    ui->registeredSensors->horizontalHeader()->resizeSection(RegisteredSensorsModel::RecordCol, 80);
    ui->registeredSensors->horizontalHeader()->resizeSection(RegisteredSensorsModel::DeleteCol, 30);
    //ui->registeredSensors->horizontalHeader()->setSectionResizeMode(RegisteredSensorsModel::DeleteCol, QHeaderView::ResizeToContents);
    ui->registeredSensors->horizontalHeader()->setSectionResizeMode(RegisteredSensorsModel::AddressCol, QHeaderView::Stretch);
    ui->registeredSensors->horizontalHeader()->setSectionResizeMode(RegisteredSensorsModel::NameEditCol, QHeaderView::Stretch);
    ui->registeredSensors->horizontalHeader()->setSectionResizeMode(RegisteredSensorsModel::TransfCol, QHeaderView::Stretch);
    ui->registeredSensors->horizontalHeader()->setSectionResizeMode(RegisteredSensorsModel::PrefixLogCol, QHeaderView::Stretch);

    ui->registeredSensors->setAlternatingRowColors(true);
    ui->registeredSensors->setStyleSheet(/*"QTableView::item:hover {background-color: rgb(185, 210, 235);}*/"QTableView{alternate-background-color: #f3f3f3; background-color: #ffffff;}");

    RegisteredSensorsDelegate * delegate = new RegisteredSensorsDelegate(RegisteredSensorsModel::ConfigCol, ui->registeredSensors);
    ComboBoxDelegate *comboDelegate = new ComboBoxDelegate(RegisteredSensorsModel::TypeCol, ui->registeredSensors);
    connect(delegate, SIGNAL(buttonClicked(QModelIndex&)), this, SLOT(on_registeredSensorButtonClick(QModelIndex&)));
    ui->registeredSensors->setItemDelegateForColumn(RegisteredSensorsModel::ConfigCol, delegate);
    ui->registeredSensors->setItemDelegateForColumn(RegisteredSensorsModel::TypeCol, comboDelegate);
    connect(m_sensorInputsModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(on_availableSensorsValueChanged()));
    connect(ui->registeredSensors, SIGNAL(clicked(QModelIndex)), this, SLOT(on_registeredSensorClicked(QModelIndex)));

    foreach(CompactRio::NamedAddress as, CompactRio::instance()->availableInputs()){
        SensorInputItem * sensor = new SensorInputItem(QString::number(as.address), as.name);
        m_sensorInputsModel->addInput(sensor);
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

/**
 * SLOT called when "nb of graphs" value has changed. Update this value in config and redraw addresses config
 * @brief MainWindow::on_graphNbValueChanged
 * @param nb The new value for graph number
 */
void MainWindow::on_graphNbValueChanged(int nb)
{
    //sensorConfig->updateDisplayGraphList(nb);
}

void MainWindow::on_serialPortValueChanged(QString portName)
{
    //qDebug() << "port name: " << portName;
    sensorConfig->initializeSerialPort(portName);
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
            RegisteredSensorItem *parent = NULL;
            if(sensor->parent()){
                for(int i=0;i<m_registeredSensorsModel->items().count(); ++i){
                    if(m_registeredSensorsModel->items().at(i)->sensorInpurItem() == sensor->parent()){
                        parent = m_registeredSensorsModel->items().at(i);
                        break;
                    }
                }
            }
            m_registeredSensorsModel->addSensor(new RegisteredSensorItem(sensor), parent);
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
    fileHelper->createLogFiles();
}

void MainWindow::on_registeredSensorClicked(QModelIndex index)
{
    if(index.column() == RegisteredSensorsModel::DeleteCol && (index.flags() & Qt::ItemIsEnabled)){
        QSortFilterProxyModel * proxyModel = static_cast<QSortFilterProxyModel *>(ui->registeredSensors->model());
        QModelIndex realIndex = proxyModel->mapToSource(index);
        m_registeredSensorsModel->removeSensor(realIndex);
    }
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

void MainWindow::on_loadWPBtnClicked()
{
    QString fileName = ui->listViewWPFiles->selectionModel()->selectedIndexes().first().data().toString();
    QList<QPointF> waypoints = fileHelper->loadWaypointsFile(fileName);
    foreach (QPointF wp, waypoints) {
        drawWayPointOnMap(wp, false);
    }
    addStatusText("Waypoints ["+ QString::number(waypoints.length()) +"] have been loaded ! \n");
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

void MainWindow::on_saveConfig()
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::ReadWrite);
    ds << *SensorTypeManager::instance();
    ds << *SensorConfig::instance();
    //qDebug() << "Serialization test : ba ="<<ba.toHex();
    bool b = DatabaseManager::instance()->insertSensorConfigBlob(ba.toHex());
    if (b) {
        addStatusText("Config has been saved !\n");
    } else {
        addStatusText("[WARNING] Config could not be saved !\n");
    }
}

void MainWindow::on_loadConfig()
{
    //QByteArray ba = QByteArray::fromHex("000000070000000e0043006f006d00700061007300730000001800470050005300200050006f0073006900740069006f006e00000012004700500053002000530070006500650064000000140052006100640069006f006d00650074006500720000001600540065006d007000650072006100740075007200650000001c00570069006e006400200044006900720065006300740069006f006e0000001400570069006e00640020005300700065006500640000000400000004003600350000001600530065007200690061006c0020004d003000500031010000000e0055006e006b006e006f0077006e0001000000260050005200690073006d0065002000440061007400610020004400650063006f0064006500720000000000000000000000000000000e00360035002e0041004400430031000000080041004400430031010000001400570069006e0064002000530070006500650064010100000006005f005f005f000000000000000000000000000000040039003600000014005000540031003000300020004d003000500030010000001600540065006d007000650072006100740075007200650101000000200050005400310030003000200063006f006e00760065007200740069006f006e00000000000000000000000000000004003900370000001c005000540031003000300020004d003000500031005f003100300030010000001400570069006e00640020005300700065006500640101000000200050005400310030003000200063006f006e00760065007200740069006f006e00000000000000000000000000000003000000000000000000000001000000000000000200000000");
    QByteArray blob = DatabaseManager::instance()->getSensorConfigBlob(0); // TODO - change fixed mission id
    QByteArray ba = QByteArray::fromHex(blob);
    QDataStream ds(&ba, QIODevice::ReadWrite);
    loadProfile(ds);
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
    QList<QString> sensorTypeList;
    for ( int i = 0 ; i < ui->listViewData->model()->rowCount() ; ++i ) {
      QString missionName = ui->listViewData->model()->index( i, 0 ).data( Qt::DisplayRole ).toString() ;
      sensorTypeList.append(missionName);
    }
    //qDebug() << "on_exportBtnClicked() - " << missionName << ", " << datatype;
    dataExporter->exportData(missionName, sensorTypeList);
}

void MainWindow::on_removeMissionBtnClicked()
{
    QString missionName = ui->listViewMission->selectionModel()->selectedIndexes().first().data().toString();
    QMessageBox msgBox;
    msgBox.setText("You are about to do some irreversible changes.");
    msgBox.setInformativeText("Do you really want to delete mission: "+ missionName +" ?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();
    switch (ret) {
        case QMessageBox::Yes:
            // Yes was clicked
            if (missionName == dbManager->getCurrentMissionName()) {
                addStatusText("[Warning] Current mission can not be deleted ! \n");
                return;
            }
            dbManager->removeMission(missionName);
            //qDebug() << "on_removeMissionBtnClicked() - " << missionName << " - "<< b;
            updateMissionList();
            break;
        case QMessageBox::No:
            // No was clicked, do nothing
            break;
        default:
            // should never be reached
            break;
    }
}

void MainWindow::on_removeAllMissionBtnClicked()
{
    //qDebug() << "on_removeAllMissionBtnClicked() " << ui->listViewMission->selectionModel()->model()->rowCount();
    QMessageBox msgBox;
    msgBox.setText("You are about to do some irreversible changes.");
    msgBox.setInformativeText("Do you really want to delete ALL missions ?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();
    switch (ret) {
        case QMessageBox::Yes:
            // Yes was clicked
            for ( int i = 0 ; i < ui->listViewMission->model()->rowCount() ; ++i ) {
              QString missionName = ui->listViewMission->model()->index( i, 0 ).data( Qt::DisplayRole ).toString() ;
              if (missionName != dbManager->getCurrentMissionName()) {
                  // remove all missions except current one
                  dbManager->removeMission(missionName);
              }
            }
            updateMissionList();
            break;
        case QMessageBox::No:
            // No was clicked, do nothing
            break;
        default:
            // should never be reached
            break;
    }
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

void MainWindow::on_sensorTypeRequestDone(int statusCode, QList<QString> sensorTypes)
{
    if (statusCode == 200) {
        qDebug() << "sensor types received !";
        foreach (QString stName, sensorTypes) {
            SensorTypeManager::instance()->createType(stName);
        }
        // TODO refresh panels with sensor types combo box
    } else {
        qDebug() << "[ERROR] while getting the sensor types from server";
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
    compactRio->getPRismeSyncTimestamp();
    for(quint8 i=0;i<8;++i){
        compactRio->setPRismeSamplingRate(10, i);
    }
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
    } else if(ui->tabWidget->widget(tabIndex)->objectName() == "navsys_tab") {
        updateWPFileList();
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
 * @param hasClicked Indicates if the user right-clicked on map to draw the point (in manual mode, points can not be drawn with right click)
 */
void MainWindow::drawWayPointOnMap(QPointF newPoint, bool hasClicked)
{
    if (!hasClicked || ui->navModeComboBox->currentIndex() == 1)
    {
        //qDebug() << "drawWayPointOnMap()" << newPoint << " - length: "<< wayPoints.length();
        QPoint previousPoint;
        if (!wayPoints.empty())
            previousPoint = ((PointOnMap)wayPoints.last()).p;
        QPointF pointFloat;
        QPoint pointOnMap;
        if (hasClicked) {
            // if user clicked, point needs to be mapped to scene
            pointFloat = ui->graphicsView->mapToScene(newPoint.toPoint());
        } else {
            pointFloat = newPoint;
        }
        pointOnMap = pointFloat.toPoint();
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

void MainWindow::loadProfile(QDataStream &ds)
{
    ds >> *SensorTypeManager::instance();
    ds >> *SensorConfig::instance();
    /*qDebug() << "Loaded SensorTypes : ";
    foreach(const QString &s, SensorTypeManager::instance()->list()){
        qDebug() << "\t" << s;
    }
    qDebug() << "Loaded Sensors : ";
    foreach(const Sensor* s, SensorConfig::instance()->getSensors()){
        qDebug() << "\t" << s->address() << ", " << s->name();
    }*/

    buildConfigSensorsView();
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

void MainWindow::buildConfigSensorsView()
{
    QList<Sensor *> list = SensorConfig::instance()->getSensors();
    int i=0;
    bool ctn = true;
    while(!list.isEmpty()){
        ctn = true;
        int idx = i%list.count();
        Sensor* s = list.at(idx);
        SensorAddress sa(s->address());
        SensorInputItem *si = m_sensorInputsModel->getItem(sa);
        if(s->isData()){
            if(!si){
//                // The sensorInputItem does not exist for this sensor, so we create it
//                // We search for its parent;
//                if(sa.hasParent()){
//                    SensorInputItem *sip = m_sensorInputsModel->getItem(sa.parent());
//                    if(sip){
//                        si = new SensorInputItem(sa.id());
//                        sip->addChild(si);
//                    }else{
//                        ctn = false;
//                    }
//                }else{
                    ctn = false;
//                }
            }
            // If si exist or created
            if(ctn){
                RegisteredSensorItem *rsi = m_registeredSensorsModel->getItem(sa);
                if(rsi){
                    // The RegisteredSensorItem already exist : so ew can remove the sensor from the list
                    list.removeAt(idx);
                }else{
                    // We try to create the rsi
                    if(sa.hasParent()){
                        RegisteredSensorItem *rsip = m_registeredSensorsModel->getItem(sa.parent());
                        if(rsip){
                            rsi = new RegisteredSensorItem(si, s);
                            m_registeredSensorsModel->addSensor(rsi, rsip);
                        }else{
                            ctn = false;
                        }
                    }else{
                        rsi = new RegisteredSensorItem(si, s);
                        m_registeredSensorsModel->addSensor(rsi);
                    }
                }
            }
            if(!ctn){
                ++i;
            }
        }else{
            list.removeAt(idx);
        }
    }
    ui->registeredSensors->sortByColumn(0);
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
    //int nbPlots = sensorConfig->getDisplayValues().size();
    int nbPlots = ui->graphNbSpinBox->value();
    //ui->graphNbSpinBox->setValue(nbPlots);
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

    //QList<Sensor*> sensorsToPlot = sensorConfig->getSensorsForPlot(plotIndex+1); // plotIndex + 1 because index 0 is for "NO" plot
    if (true/*sensorsToPlot.length() > 0*/)
    {
        DataPlot* dataPlot = new DataPlot(plotIndex, this);
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
    QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
    foreach (QSerialPortInfo sp, list) {
        ui->serialPortsComboBox->addItem(sp.portName());
    }
    connect(ui->serialPortsComboBox, SIGNAL(activated(QString)), this, SLOT(on_serialPortValueChanged(QString)));
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

    connect(ui->exportBtn, SIGNAL(clicked()), this, SLOT(on_exportBtnClicked()));
    connect(ui->removeMissionBtn, SIGNAL(clicked()), this, SLOT(on_removeMissionBtnClicked()));
    connect(ui->removeAllMissionBtn, SIGNAL(clicked()), this, SLOT(on_removeAllMissionBtnClicked()));
    connect(ui->backendAddressField, SIGNAL(textChanged(QString)), this, SLOT(on_backendAddressValueChanged(QString)));
    connect(httpRequester, SIGNAL(pingRequestDone(int)), this, SLOT(on_pingRequestDone(int)));
    connect(dataExporter, SIGNAL(displayInGui(QString)), this, SLOT(addStatusText(QString)));

    httpRequester->sendPingRequest();
    QTimer *timer = new QTimer();
    timer->setInterval(5000);
    connect(timer, SIGNAL(timeout()), httpRequester, SLOT(sendPingRequest()));
    timer->start();

    HttpRequester::instance()->setBackendAddress(ui->backendAddressField->text());
}

void MainWindow::displayDataForMission(QString missionName)
{
    QStandardItemModel *model = dbManager->getDataForMissionsAsModel(missionName);
    ui->listViewData->setModel(model);
    //ui->listViewData->setCurrentIndex(model->index(0,0));
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
    connect(ui->listViewMission->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(on_missionSelectedChanged(QItemSelection)));
}

/**
 * Update the list of waypoint files
 * @brief MainWindow::updateWPFileList
 */
void MainWindow::updateWPFileList()
{
    QStandardItemModel* model = new QStandardItemModel;
    QString folderPath = QDir::currentPath() + "/waypoints";
    QDir myDir(folderPath);
    QFileInfoList filesList = myDir.entryInfoList(QStringList("*"));
    foreach (QFileInfo fi, filesList) {
        if (fi.isFile()) {
            model->appendRow(new QStandardItem(fi.fileName()));
        }
    }
    ui->listViewWPFiles->setModel(model);
    ui->listViewWPFiles->setCurrentIndex(model->index(0,0));
}
