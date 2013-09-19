#include "sensortransformationconfig.h"
#include "ui_sensortransformationconfig.h"

SensorTransformationConfig::SensorTransformationConfig(RegisteredSensorItem *sensor, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SensorTransformationConfig)
{
    ui->setupUi(this);
    ui->widget->setWindowTitle("Sensor ["+ sensor->name() +"] : Transformation configuration");

    QList<TransformationBaseClass *> transformationList = TransformationManager::instance()->getTransformations();
    for(int i=0;i<transformationList.count();++i){
        QString name = transformationList[i]->getTransformationDefinition().name;
        QStandardItemModel *model = new QStandardItemModel();
        model->setItem(i,new QStandardItem(name));
        ui->comboBox->setModel(model);
    }

}

SensorTransformationConfig::~SensorTransformationConfig()
{
    delete ui;
}
