#include "addtoplotwidget.h"
#include "ui_addtoplotwidget.h"
#include <QPushButton>
#include "sensorConfig.h"
#include "model/sensor.h"
#include <QListWidgetItem>

#include <QDebug>

AddToPlotWidget::AddToPlotWidget(int plotNumber, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddToPlotWidget),
    m_plotNumber(plotNumber)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Tool);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(QString("Plot n°%1: Sensors selection.").arg(plotNumber));

    connect(ui->buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(on_save()));
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()), this, SLOT(on_reset()));
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SIGNAL(cancelButtonClicked()));

    fillSensorList();
}

AddToPlotWidget::~AddToPlotWidget()
{
    delete ui;
}

void AddToPlotWidget::fillSensorList()
{
    ui->plotList->clear();

    foreach(Sensor *s, SensorConfig::instance()->getSensors()){
        if(s->isData()){
            bool checked = SensorConfig::instance()->sensorForPlot(m_plotNumber).contains(s);
            QListWidgetItem *item = new QListWidgetItem(s->name(), ui->plotList);
            item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            item->setCheckState(checked?Qt::Checked:Qt::Unchecked);
            item->setData(Qt::UserRole+100, QVariant::fromValue((void *)s));
        }
    }
}

void AddToPlotWidget::on_save()
{
    bool changed = false;

    for(int i=0;i<ui->plotList->count();++i){
        QListWidgetItem * wi = ui->plotList->item(i);
        bool checked = wi->checkState() == Qt::Checked;
        Sensor *s = (Sensor *)wi->data(Qt::UserRole+100).value<void *>();
        if(checked != SensorConfig::instance()->sensorForPlot(m_plotNumber).contains(s)){
            changed = true;
        }
        if(checked){
            SensorConfig::instance()->addSensorToPlot(m_plotNumber, s);
        }else{
            SensorConfig::instance()->removeSensorFromPlot(m_plotNumber, s);
        }
    }
    emit savedButtonClicked(changed);
    this->close();
}

void AddToPlotWidget::on_reset()
{
    fillSensorList();
    emit resetButtonClicked();
}
