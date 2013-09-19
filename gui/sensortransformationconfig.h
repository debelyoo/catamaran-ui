#ifndef SENSORTRANSFORMATIONCONFIG_H
#define SENSORTRANSFORMATIONCONFIG_H

#include <QWidget>
#include <QStandardItem>
#include "gui/model/registeredsensoritem.h"
#include "transformation/transformationmanager.h"

namespace Ui {
class SensorTransformationConfig;
}

class SensorTransformationConfig : public QWidget
{
    Q_OBJECT

public:
    explicit SensorTransformationConfig(RegisteredSensorItem *sensor, QWidget *parent = 0);
    ~SensorTransformationConfig();

private:
    Ui::SensorTransformationConfig *ui;
};

#endif // SENSORTRANSFORMATIONCONFIG_H
