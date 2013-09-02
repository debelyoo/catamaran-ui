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

    createPlotsPanel();

    createConfigurationPanel();


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
    sendEngineCommand();
}

void MainWindow::on_directionValueChanged(int val)
{
    ui->directionSlider->setValue(val);
    ui->directionSpinBox->setValue(val);
    updateLeftRightSliders();
    sendEngineCommand();
}

void MainWindow::on_saveConfigClicked()
{
    // write config file
    fileHelper->writeFile("config2.txt", sensorConfig->getSensorsAsTabSeparatedText());
    addStatusText("Config saved !\n");
    changeSaveBtnColor("gray");
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
    s->setType(typeInd);
    QString msg = "Type ["+ QString::number(rowIndex) +"] changed: " + sensorConfig->getSensorTypes().value(typeInd) + "\n";
    addStatusText(msg);
    changeSaveBtnColor("red");
}

void MainWindow::on_displayValueChanged(QString dis, int rowIndex)
{
    Sensor* s = sensorConfig->getSensors()[rowIndex];
    s->setDisplay(dis);
    QString msg = "Display ["+ QString::number(rowIndex) +"] changed: " + dis + "\n";
    addStatusText(msg);
}

void MainWindow::on_recordValueChanged(bool rec, int rowIndex)
{
    Sensor* s = sensorConfig->getSensors()[rowIndex];
    s->setRecord(rec);
    QString msg = "Record ["+ QString::number(rowIndex) +"] changed: " + QString::number(rec) + "\n";
    addStatusText(msg);
}

void MainWindow::on_streamValueChanged(bool stream, int rowIndex)
{
    Sensor* s = sensorConfig->getSensors()[rowIndex];
    s->setStream(stream);
    QString msg = "Stream ["+ QString::number(rowIndex) +"] changed: " + QString::number(stream) + "\n";
    addStatusText(msg);
}

void MainWindow::on_filenameValueChanged(QString fn, int rowIndex)
{
    QString msg = "Filename ["+ QString::number(rowIndex) +"] changed: " + fn + "\n";
    addStatusText(msg);
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
    /*char str[128];
    sprintf(str, "drawPointOnMap() [%f,%f]\n", x, y);
    printf(str);
    fflush(stdout);*/
    double rad = 1;
    QColor color = QColor(255, 90, 0); // orange
    ui->graphicsView->scene()->addEllipse(x-rad, y-rad, rad, rad, QPen(color), QBrush(Qt::SolidPattern));
    // center view on new point
    ui->graphicsView->centerOn(x, y);
}


/// private methods

void MainWindow::sendEngineCommand()
{
    if (!sliderIsMoving)
    {
        /*char str[128];
        sprintf(str, "sendEngineCommand()\n");
        printf(str);
        fflush(stdout);*/
        sendLeftEngineCommand();
        sendRightEngineCommand();
    }
}

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
    sprintf(str, "sendLeftEngineCommand() [%d]\n", correctEngineCommandValue(val));
    printf(str);
    fflush(stdout);
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
    sprintf(str, "sendRightEngineCommand() [%d]\n", correctEngineCommandValue(val));
    printf(str);
    fflush(stdout);
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
    QMap<int, QString> sensorTypes = sensorConfig->getSensorTypes();
    for (int i=0; i < sensorTypes.size(); i++)
    {

        typeBox->addItem(sensorTypes.value(i));
        //typeBox->addItem(sensorTypes.at(i));
    }
    QWidget* typeBoxContainer = createSpacedWidget(typeBox, 0, 10);
    InRowComboBox* displayBox = new InRowComboBox(this, sensorIndex);
    displayBox->addItem("NO");
    for (int i=0; i < 3; i++)
    {
        char str[128];
        sprintf(str, "G%d", i);
        displayBox->addItem(QString(str));
    }
    displayBox->setCurrentText(s->getDisplay());
    QWidget* displayBoxContainer = createSpacedWidget(displayBox, 0, 10);
    InRowCheckBox* recordCB = new InRowCheckBox(this, sensorIndex);
    recordCB->setChecked(s->getRecord());
    QWidget* recordCBContainer = createSpacedWidget(recordCB, 10, 0);
    InRowCheckBox* streamCB = new InRowCheckBox(this, sensorIndex);
    streamCB->setChecked(s->getStream());
    QWidget* streamCBContainer = createSpacedWidget(streamCB, 10, 0);
    InRowLineEdit* filenameField = new InRowLineEdit(this, sensorIndex);
    filenameField->setText(s->getFilename());
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
    connect(displayBox, SIGNAL(valueChanged(QString,int)), this, SLOT(on_displayValueChanged(QString,int)));
    connect(recordCB, SIGNAL(clicked(bool,int)), this, SLOT(on_recordValueChanged(bool,int)));
    connect(streamCB, SIGNAL(clicked(bool,int)), this, SLOT(on_streamValueChanged(bool,int)));
    connect(filenameField, SIGNAL(textChanged(QString, int)), this, SLOT(on_filenameValueChanged(QString, int)));
}

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
        if (i == 0)
            plot = createPlotByDate(0, i * plotHeight, viewport->width(), plotHeight);
        else
            plot = createPlot(0, i * plotHeight, viewport->width(), plotHeight);
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

QWidget* MainWindow::createPlotByDate(int xPos, int yPos, int width, int height)
{
    QWidget *viewport = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    viewport->setLayout(layout);

    QCustomPlot* customPlot = new QCustomPlot;
    layout->addWidget(customPlot);
    customPlot->setGeometry(xPos, yPos, width, height);

    // set locale to english, so we get english month names:
    customPlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));
    // seconds of current time, we'll use it as starting point in time for data:
    double now = QDateTime::currentDateTime().toTime_t();
    srand(8); // set the random seed, so we always get the same random data
    // create multiple graphs:
    for (int gi=0; gi<5; ++gi)
    {
    customPlot->addGraph();
    QPen pen;
    pen.setColor(QColor(0, 0, 255, 200));
    customPlot->graph()->setLineStyle(QCPGraph::lsLine);
    customPlot->graph()->setPen(pen);
    customPlot->graph()->setBrush(QBrush(QColor(255/4.0*gi,160,50,150)));
    // generate random walk data:
    QVector<double> time(250), value(250);
    for (int i=0; i<250; ++i)
    {
      time[i] = now + 24*3600*i;
      if (i == 0)
        value[i] = (i/50.0+1)*(rand()/(double)RAND_MAX-0.5);
      else
        value[i] = fabs(value[i-1])*(1+0.02/4.0*(4-gi)) + (i/50.0+1)*(rand()/(double)RAND_MAX-0.5);
    }
    customPlot->graph()->setData(time, value);
    }
    // configure bottom axis to show date and time instead of number:
    customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    customPlot->xAxis->setDateTimeFormat("MMMM\nyyyy");
    // set a more compact font size for bottom and left axis tick labels:
    customPlot->xAxis->setTickLabelFont(QFont(QFont().family(), 8));
    customPlot->yAxis->setTickLabelFont(QFont(QFont().family(), 8));
    // set a fixed tick-step to one tick per month:
    customPlot->xAxis->setAutoTickStep(false);
    customPlot->xAxis->setTickStep(2628000); // one month in seconds
    customPlot->xAxis->setSubTickCount(3);
    // apply manual tick and tick label for left axis:
    customPlot->yAxis->setAutoTicks(false);
    customPlot->yAxis->setAutoTickLabels(false);
    customPlot->yAxis->setTickVector(QVector<double>() << 5 << 55);
    customPlot->yAxis->setTickVectorLabels(QVector<QString>() << "Not so\nhigh" << "Very\nhigh");
    // set axis labels:
    customPlot->xAxis->setLabel("Date");
    customPlot->yAxis->setLabel("Random wobbly lines value");
    // make top and right axes visible but without ticks and labels:
    customPlot->xAxis2->setVisible(true);
    customPlot->yAxis2->setVisible(true);
    customPlot->xAxis2->setTicks(false);
    customPlot->yAxis2->setTicks(false);
    customPlot->xAxis2->setTickLabels(false);
    customPlot->yAxis2->setTickLabels(false);
    // set axis ranges to show all data:
    customPlot->xAxis->setRange(now, now+24*3600*249);
    customPlot->yAxis->setRange(0, 60);
    // show legend:
    customPlot->legend->setVisible(true);
    return viewport;
}

void MainWindow::createConfigurationPanel()
{
    // load config file
    fileHelper->loadConfigFile(sensorConfig);
    // create log files - TODO
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
