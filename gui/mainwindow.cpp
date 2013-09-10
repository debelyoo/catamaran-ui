#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qmath.h>
#include <QGraphicsPixmapItem>
#include <QGroupBox>
#include <QLineEdit>
#include <QComboBox>
#include "inRowLineEdit.h"
#include "inRowComboBox.h"
#include "inRowCheckBox.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    zoomStep = 10;
    sensorConfig = SensorConfig::instance();
    fileHelper = FileHelper::instance();
    converter = ByteArrayConverter::instance();

    ui->setupUi(this);


    // load map in graphics view (for GPS points)
    QGraphicsScene *scene = new QGraphicsScene();
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(QPixmap(":/images/Map.jpg"));
    item->setPos(0, 0);
    scene->addItem(item);
    ui->graphicsView->setScene(scene);

    //ui->tabWidget->widget(0)->installEventFilter(new KeyPressHandler());

    createConfigurationPanel();
    createPlotsPanel(); // need to be after configuration panel for plots


    // start server
    s = new Server(this);
    QObject::connect(s, SIGNAL(displayInGui(QString)), this, SLOT(addStatusText(QString)));
    s->listen();
    sliderIsMoving = false;

    QObject::connect(s, SIGNAL(gpsPointReceived(double, double)), this, SLOT(drawPointOnMap(double,double)));

    // engines' sliders & spinboxes
    QObject::connect(ui->leftSlider, SIGNAL(valueChanged(int)), ui->leftSpinBox, SLOT(setValue(int)));
    QObject::connect(ui->rightSlider, SIGNAL(valueChanged(int)), ui->rightSpinBox, SLOT(setValue(int)));
    QObject::connect(ui->leftSpinBox, SIGNAL(valueChanged(int)), ui->leftSlider, SLOT(setValue(int)));
    QObject::connect(ui->rightSpinBox, SIGNAL(valueChanged(int)), ui->rightSlider, SLOT(setValue(int)));

    // speed and direction sliders & spinboxes
    QObject::connect(ui->speedSlider, SIGNAL(valueChanged(int)), this, SLOT(on_speedValueChanged(int)));
    QObject::connect(ui->directionSlider, SIGNAL(valueChanged(int)), this, SLOT(on_directionValueChanged(int)));
    QObject::connect(ui->speedSpinBox, SIGNAL(valueChanged(int)), this, SLOT(on_speedValueChanged(int)));
    QObject::connect(ui->directionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(on_directionValueChanged(int)));
    ui->speedSpinBox->installEventFilter(new MouseClickHandler(this));
    ui->directionSpinBox->installEventFilter(new MouseClickHandler(this));

    QObject::connect(ui->speedSlider, SIGNAL(sliderPressed()), this, SLOT(on_sliderPressed()));
    QObject::connect(ui->directionSlider, SIGNAL(sliderPressed()), this, SLOT(on_sliderPressed()));
    QObject::connect(ui->speedSlider, SIGNAL(sliderReleased()), this, SLOT(on_sliderReleased()));
    QObject::connect(ui->directionSlider, SIGNAL(sliderReleased()), this, SLOT(on_sliderReleased()));

    // map view
    connect(ui->zoomInBtn, SIGNAL(clicked()), this, SLOT(zoomIn()));
    connect(ui->zoomOutBtn, SIGNAL(clicked()), this, SLOT(zoomOut()));
    connect(ui->zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    qDebug() << event->text();
}

void MainWindow::addStatusText(QString msg)
{
    ui->statusText->insertPlainText(msg);
    QTextCursor c =  ui->statusText->textCursor();
    c.movePosition(QTextCursor::End);
    ui->statusText->setTextCursor(c);
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

void MainWindow::on_speedValueChanged(int val)
{
    ui->speedSlider->setValue(val);
    ui->speedSpinBox->setValue(val);
    updateLeftRightSliders();
    //sendEngineCommand();
}

void MainWindow::on_directionValueChanged(int val)
{
    ui->directionSlider->setValue(val);
    ui->directionSpinBox->setValue(val);
    updateLeftRightSliders();
    //sendEngineCommand();
}

void MainWindow::on_saveConfigClicked()
{
    // write config file
    fileHelper->writeFile("config2.txt", sensorConfig->getSensorsAsTabSeparatedText(), false);
    fileHelper->createLogFiles(sensorConfig);
    addStatusText("Config saved !\n");
    changeSaveBtnColor("gray");
    // recreate plots panel
    clearPlotsPanel();
    createPlotsPanel();
}

void MainWindow::on_addressValueChanged(QString addrStr, int rowIndex)
{
    Sensor* s = sensorConfig->getSensors()[rowIndex];
    bool b;
    int addr = addrStr.toInt(&b);
    if (b)
    {
        s->setAddress(addr);
        QString msg = "Address ["+ QString::number(rowIndex) +"] changed: " + addrStr + "\n";
        addStatusText(msg);
    }
    else
    {
        QString msg = "Address: " + addrStr + " - Bad format !\n";
        addStatusText(msg);
    }
}

void MainWindow::on_nameValueChanged(QString name, int rowIndex)
{
    Sensor* s = sensorConfig->getSensors()[rowIndex];
    s->setName(name);
    QString msg = "Name ["+ QString::number(rowIndex) +"] changed: " + name + "\n";
    addStatusText(msg);
    changeSaveBtnColor("red");
}

void MainWindow::on_typeValueChanged(int typeInd, int rowIndex)
{
    Sensor* s = sensorConfig->getSensors()[rowIndex];
    s->setType(sensorConfig->getSensorTypes().value(typeInd));
    QString msg = "Type ["+ QString::number(rowIndex) +"] changed: " + sensorConfig->getSensorTypes().value(typeInd)->getName() + "\n";
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
    ui->graphicsView->scene()->addEllipse(x-rad, y-rad, rad, rad, QPen(color), QBrush(Qt::SolidPattern));
    // center view on new point
    ui->graphicsView->centerOn(x, y);
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
    int val = ui->leftSlider->value();
    quint8 command = MessageUtil::Set;
    quint8 engineAddr = 7;
    QByteArray data;
    data.push_back(command);
    data.push_back(converter->intToByteArray(2, 4)); // array len
    data.push_back(converter->byteArrayForCmdParameterInt(engineAddr));
    data.push_back(converter->byteArrayForCmdParameterInt(val));
    s->sendCommandMessage(data);
    char str[128];
    sprintf(str, "sendLeftEngineCommand() [%d]", correctEngineCommandValue(val));
    qDebug() << str;
}

void MainWindow::sendRightEngineCommand()
{
    int val = ui->rightSlider->value();
    quint8 command = MessageUtil::Set;
    quint8 engineAddr = 8;
    QByteArray data;
    data.push_back(command);
    data.push_back(converter->intToByteArray(2, 4));
    data.push_back(converter->byteArrayForCmdParameterInt(engineAddr));
    data.push_back(converter->byteArrayForCmdParameterInt(val));
    s->sendCommandMessage(data);
    char str[128];
    sprintf(str, "sendRightEngineCommand() [%d]", correctEngineCommandValue(val));
    qDebug() << str;
}

/**
 * Correct the value for the engine command. In UI range is from -100 to 100, but engines take values between -127 to 127.
 * @brief MainWindow::correctEngineCommandValue
 * @param val
 * @return
 */
int MainWindow::correctEngineCommandValue(int val)
{
   return val * 1.27;
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
    addressField->setText(QString::number(s->getAddress()));
    addressField->setFixedWidth(30);
    QWidget* addressFieldContainer = createSpacedWidget(addressField, 0, 30);
    InRowLineEdit* nameField = new InRowLineEdit(this, sensorIndex);
    nameField->setText(QString(s->getName()));
    InRowComboBox* typeBox = new InRowComboBox(this, sensorIndex);
    QMap<int, SensorType*> sensorTypes = sensorConfig->getSensorTypes();
    for (int i=0; i < sensorTypes.size(); i++)
    {
        typeBox->addItem(sensorTypes.value(i)->getName());
    }
    typeBox->setCurrentIndex(s->getType()->getId());
    QWidget* typeBoxContainer = createSpacedWidget(typeBox, 0, 10);
    InRowComboBox* displayBox = new InRowComboBox(this, sensorIndex);
    QMap<int, QString> displayGraphs = sensorConfig->getDisplayValues();
    for (int i=0; i < displayGraphs.size(); i++)
    {
        displayBox->addItem(displayGraphs.value(i));
    }
    displayBox->setCurrentIndex(s->getDisplay());
    QWidget* displayBoxContainer = createSpacedWidget(displayBox, 0, 10);
    InRowCheckBox* recordCB = new InRowCheckBox(this, sensorIndex);
    recordCB->setChecked(s->getRecord());
    QWidget* recordCBContainer = createSpacedWidget(recordCB, 10, 0);
    InRowCheckBox* streamCB = new InRowCheckBox(this, sensorIndex);
    streamCB->setChecked(s->getStream());
    QWidget* streamCBContainer = createSpacedWidget(streamCB, 10, 0);
    InRowLineEdit* filenameField = new InRowLineEdit(this, sensorIndex);
    filenameField->setText(s->getLogFilePrefix());
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
    connect(typeBox, SIGNAL(valueChanged(int,int)), this, SLOT(on_typeValueChanged(int, int)));
    connect(displayBox, SIGNAL(valueChanged(int,int)), this, SLOT(on_displayValueChanged(int,int)));
    connect(recordCB, SIGNAL(clicked(bool,int)), this, SLOT(on_recordValueChanged(bool,int)));
    connect(streamCB, SIGNAL(clicked(bool,int)), this, SLOT(on_streamValueChanged(bool,int)));
    connect(filenameField, SIGNAL(textChanged(QString, int)), this, SLOT(on_filenameValueChanged(QString, int)));
}

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
    int nbPlots = 3;
    int plotHeight = round((ui->tabWidget->height() - 50) / (double)2);
    for (int i = 0; i < nbPlots; i++)
    {
        //QLabel* l = new QLabel("Graph X");
        //layout->addWidget(l);
        QWidget* plot;
        QRect geom = QRect(0, i * plotHeight, viewport->width(), plotHeight);
        plot = createPlotByDate(i, geom);
        layout->addWidget(plot);
    }
    viewport->setGeometry(0,0,ui->tabWidget->width()-80, nbPlots * plotHeight);
    QVBoxLayout *plotsPanelLayout = new QVBoxLayout;
    plotsPanel->setLayout(plotsPanelLayout);
    scrollArea->setWidget(viewport);
    plotsPanelLayout->addWidget(scrollArea);
}

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
        dataPlot->setGeometry(geometry);
        QTimer *timer = new QTimer();
        timer->setInterval(3000);
        connect(timer, SIGNAL(timeout()), dataPlot, SLOT(updatePlot()));
        timer->start();
        layout->addWidget(dataPlot);
    }
    return viewport;
}

void MainWindow::createConfigurationPanel()
{
    // load sensorTypes file
    fileHelper->loadSensorTypesFile(sensorConfig);
    // load config file
    fileHelper->loadConfigFile(sensorConfig);
    // create log files
    fileHelper->createLogFiles(sensorConfig);
    QList<Sensor*> sensors = sensorConfig->getSensors();

    QWidget *configurationPanel = ui->tabWidget->widget(2);
    QScrollArea *scrollArea = new QScrollArea;
    QWidget *viewport = new QWidget;
    QGridLayout *layout = new QGridLayout;
    viewport->setLayout(layout);

    //QGroupBox *addressGroupBox = new QGroupBox("Addresses");

    createLabelLine(layout);
    for (int i = 1; i <= sensors.length(); i++)
    {
        createAddressFormRow(layout, i, sensors[i-1]);
    }
    scrollArea->setWidget(viewport);

    QVBoxLayout *configurationPanelLayout = new QVBoxLayout;
    configurationPanel->setLayout(configurationPanelLayout);
    QWidget *scrollAreaContainer = createSpacedWidget(scrollArea,0,300);
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
