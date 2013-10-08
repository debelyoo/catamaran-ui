#ifndef SENSORTRANSFORMATIONCONFIG_H
#define SENSORTRANSFORMATIONCONFIG_H

#include <QWidget>
#include <QStandardItem>
#include <QSortFilterProxyModel>
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
    TransformationBaseClass *configuredTransformation();


private:
    Ui::SensorTransformationConfig *ui;
    QWidget *createSensorWidget(QWidget *parent, const TransformationBaseClass::ParameterDescription &param, int paramIndex);

    void setToolTipTexts(const TransformationBaseClass::ParameterDescription &param);
    void setToolTipTexts(const TransformationBaseClass *transformation);

    //TransformationBaseClass *m_transformation;
    QVector<TransformationBaseClass *> m_transformationList;
    int m_currentIndex;
    RegisteredSensorItem *m_linkedSensor;

private slots:
    void on_transformationSelection(const int index);
    void on_textEditorChange();
    void on_doubleEditorChange(double value);
    void on_integerEditorChange(int value);
    void on_booleantEditorChange(int value);
    void on_accept();

    void on_comboxItemEnter(QModelIndex index);
};

#endif // SENSORTRANSFORMATIONCONFIG_H
