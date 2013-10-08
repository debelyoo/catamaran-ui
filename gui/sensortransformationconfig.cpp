#include "sensortransformationconfig.h"
#include "ui_sensortransformationconfig.h"

#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>

/** Create a pop-up widget used to configure the transformation of the selected sensor. The contstrutor
 * create a new instance of all available transformations. At the end unused transformation are distroyed
 * @brief SensorTransformationConfig::SensorTransformationConfig
 * @param sensor The sensor that handle the transformation
 * @param parent The parent window
 */
SensorTransformationConfig::SensorTransformationConfig(RegisteredSensorItem *sensor, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SensorTransformationConfig)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Tool);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(QString("Sensor [%1] : Transformation configuration").arg(sensor->name()));

    m_linkedSensor = sensor;

    QList<TransformationBaseClass *> transformationList = TransformationManager::instance()->getTransformations();
    QStandardItemModel *model = new QStandardItemModel();
    //QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel();
    //proxyModel->setSourceModel(model);
    model->setItem(0,new QStandardItem("None"));
    m_transformationList.append(NULL);
    TransformationBaseClass *sensorTransformation = sensor->transformation();

    m_currentIndex = 0;

    for(int i=0;i<transformationList.count();++i){
        TransformationBaseClass *tr = transformationList[i]->newInstance();
        m_transformationList.append(tr);
        QString name = tr->getTransformationDefinition().name;
        if(sensorTransformation && name == sensorTransformation->getTransformationDefinition().name){
            tr->setParameters(sensorTransformation->getParameters());
            m_currentIndex = i+1;
        }
        model->setItem(i+1,new QStandardItem(name));
    }
    //proxyModel->sort(0);
    ui->comboBox->setModel(model);

    ui->transformationDescription->setText("");


    ui->comboBox->setCurrentIndex(m_currentIndex);
    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(on_transformationSelection(int)));
    //ui->comboBox->view()->
    connect(ui->comboBox->view(), SIGNAL(entered(QModelIndex)), this, SLOT(on_comboxItemEnter(QModelIndex)));
    on_transformationSelection(m_currentIndex);

    connect(ui->acceptButtonBox, SIGNAL(accepted()), this, SLOT(on_accept()));
    connect(ui->acceptButtonBox, SIGNAL(rejected()), this, SLOT(close()));
}

/** Destructor. Delete every valid pointer in m_transformationList
 * @brief SensorTransformationConfig::~SensorTransformationConfig
 */
SensorTransformationConfig::~SensorTransformationConfig()
{
    qDeleteAll(m_transformationList);
    delete ui;
}

/** get the resulting transformation and prevent it from destruction
 * @brief SensorTransformationConfig::configuredTransformation
 * @return
 */
TransformationBaseClass *SensorTransformationConfig::configuredTransformation()
{
    TransformationBaseClass *tmp = m_transformationList[m_currentIndex];
    m_transformationList[m_currentIndex] = NULL;
    return tmp;
}

/** Create a variant editor for a parameter
 * @brief SensorTransformationConfig::createSensorWidget
 * @param parent
 * @param param
 * @param paramIndex
 * @return a widget containing the editor and a description label
 */
QWidget *SensorTransformationConfig::createSensorWidget(QWidget *parent, const TransformationBaseClass::ParameterDescription &param, int paramIndex)
{
    QFrame * frame = new QFrame(parent);
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Plain);
    frame->setFrameStyle(1);
    QHBoxLayout *layout = new QHBoxLayout(frame);
    frame->setLayout(layout);
    QLabel *name = new QLabel(param.name + " : ", frame);
    QLabel *description = new QLabel(param.description, frame);
    description->setWordWrap(true);

    QVector<QVariant> trParams;
    if(m_transformationList[m_currentIndex]){
        trParams = m_transformationList[m_currentIndex]->getParameters();
    }

    QWidget *editor;
    if(param.typeAndDefault.type() == QVariant::String){
        QPlainTextEdit *editor2 = new QPlainTextEdit(param.typeAndDefault.toString(), frame);
        connect(editor2, SIGNAL(textChanged()), this, SLOT(on_textEditorChange()));
        editor = editor2;
    }else if(param.typeAndDefault.type() == QVariant::Double){
        QDoubleSpinBox *editor2 = new QDoubleSpinBox(frame);
        editor2->setMaximum(9999999999);
        editor2->setMinimum(-9999999999);
        editor2->setValue(trParams[paramIndex].toDouble());
        connect(editor2, SIGNAL(valueChanged(double)), this, SLOT(on_doubleEditorChange(double)));
        editor = editor2;
    }else if(param.typeAndDefault.type() == QVariant::Int){
        QSpinBox *editor2 = new QSpinBox(frame);
        editor2->setMaximum(99999999);
        editor2->setMinimum(-99999999);
        editor2->setValue(trParams[paramIndex].toInt());
        connect(editor2, SIGNAL(valueChanged(int)), this, SLOT(on_integerEditorChange(int)));
        editor = editor2;
    }else if(param.typeAndDefault.type() == QVariant::UInt){
        QSpinBox *editor2 = new QSpinBox(frame);
        editor2->setMaximum(999999999);
        editor2->setMinimum(0);
        editor2->setValue(trParams[paramIndex].toUInt());
        connect(editor2, SIGNAL(valueChanged(int)), this, SLOT(on_integerEditorChange(int)));
        editor = editor2;
    }else if(param.typeAndDefault.type() == QVariant::Char){
        QSpinBox *editor2 = new QSpinBox(frame);
        editor2->setMaximum(255);
        editor2->setMinimum(0);
        editor2->setValue(trParams[paramIndex].toChar().digitValue());
        connect(editor2, SIGNAL(valueChanged(int)), this, SLOT(on_integerEditorChange(int)));
        editor = editor2;
    }else if(param.typeAndDefault.type() == QVariant::ULongLong){
        QSpinBox *editor2 = new QSpinBox(frame);
        editor2->setMaximum(999999999);
        editor2->setMinimum(0);
        editor2->setValue(trParams[paramIndex].toULongLong());
        connect(editor2, SIGNAL(valueChanged(int)), this, SLOT(on_integerEditorChange(int)));
        editor = editor2;
    }else if(param.typeAndDefault.type() == QVariant::LongLong){
        QSpinBox *editor2 = new QSpinBox(frame);
        editor2->setMaximum(999999999);
        editor2->setMinimum(-999999999);
        editor2->setValue(trParams[paramIndex].toLongLong());
        connect(editor2, SIGNAL(valueChanged(int)), this, SLOT(on_integerEditorChange(int)));
        editor = editor2;
    }else if(param.typeAndDefault.type() == QVariant::Bool){
        QCheckBox *editor2 = new QCheckBox("on/off", frame);
        editor2->setChecked(trParams[paramIndex].toBool());
        connect(editor2, SIGNAL(stateChanged(int)), this, SLOT(on_booleanEditorChange(int)));
        editor = editor2;
    }else{
        editor = NULL;
    }

    if(editor){
        editor->setProperty("paramIndex", paramIndex);
        description->setAlignment(Qt::AlignTop);
        description->setStyleSheet( "font-style:italic;");
        name->setAlignment(Qt::AlignVCenter);
        description->setFixedHeight(32);
        editor->setFixedHeight(22);
        editor->setFixedWidth(80);
        editor->setAccessibleName(name->text());

        layout->addWidget(name, 0, Qt::AlignLeft & Qt::AlignVCenter);
        layout->addWidget(editor, 0, Qt::AlignVCenter);
        layout->addWidget(description, 1, Qt::AlignTop);

        //frame->setBaseSize()
        return frame;
    }
    return NULL;
}

/** Set tooltip texts for a parameter
 * @brief SensorTransformationConfig::setToolTipTexts
 * @param param
 */
void SensorTransformationConfig::setToolTipTexts(const TransformationBaseClass::ParameterDescription &param)
{
    Q_UNUSED(param);
}

/** Set tooltip texts for a transformation
 * @brief SensorTransformationConfig::setToolTipTexts
 * @param transformation
 */
void SensorTransformationConfig::setToolTipTexts(const TransformationBaseClass *transformation)
{
    if(!transformation){
        ui->tooltipText->setText("");
        return;
    }
    QString tooltipText =   "<b>%1 :</b><br>%2<br>%3";
    QString subsensorsText;
    QVector<TransformationBaseClass::SubSensor> subsensors = transformation->getSubSensors();
    for(int i=0;i<subsensors.count(); ++i){
        subsensorsText += QString(" %1) %2 : %3<br>").arg(QString::number(i+1), subsensors[i].name, subsensors[i].description);
    }
    if(subsensorsText.count()){
        subsensorsText = "<br><b>Subsensors :</b><br>" + subsensorsText;
    }
    ui->tooltipText->setText(tooltipText.arg(transformation->getTransformationDefinition().name, transformation->getTransformationDefinition().description, subsensorsText));
}

/** Slot called when the selection change in the transformation combo box
 * @brief SensorTransformationConfig::on_transformationSelection
 * @param index index of the selection
 */
void SensorTransformationConfig::on_transformationSelection(const int index)
{
    m_currentIndex = index;
    //TransformationBaseClass *tr = TransformationManager::instance()->getTransformation(name);
    TransformationBaseClass *tr = m_transformationList[index];
    if(tr){
        ui->transformationDescription->setText(tr->getTransformationDefinition().description);
        int i=0;
        QWidget *area = new QWidget();
        QVBoxLayout *areaLayout = new QVBoxLayout(area);
        area->setLayout(areaLayout);
        QVector<TransformationBaseClass::ParameterDescription> params = tr->getParametersDefinition();
        for(int j=0;j < params.count(); ++j){
            //qDebug() << "[" << j << "(" << i << ")/" << params.count() << "Crate new parameter widget : name = " << params[j].name;
            QWidget *sensorWidget = createSensorWidget(area, params[j], j);
            if(sensorWidget){
                //sensorWidget->setGeometry(5, 5 + 40*i, 611, 41);
                sensorWidget->setFixedSize(611, 40);
                sensorWidget->show();
                areaLayout->addWidget(sensorWidget);
                ++i;
            }
        }
        area->show();
        ui->paramsScrollArea->setWidget(area);
    }else{
        ui->transformationDescription->setText("");
    }
    setToolTipTexts(m_transformationList[m_currentIndex]);

}

void SensorTransformationConfig::on_textEditorChange()
{
    int index = sender()->property("paramIndex").toInt();

    QString text = "";
    const QPlainTextEdit* editor = static_cast<const QPlainTextEdit *>(sender());
    if(editor){
        text = editor->toPlainText();
    }
    QVector<QVariant> params = m_transformationList[m_currentIndex]->getParameters();
    params[index] = QVariant(text);
    m_transformationList[m_currentIndex]->setParameters(params);
}

void SensorTransformationConfig::on_integerEditorChange(int value)
{
    int index = sender()->property("paramIndex").toInt();
    QVector<QVariant> params = m_transformationList[m_currentIndex]->getParameters();
    params[index] = QVariant(value);
    m_transformationList[m_currentIndex]->setParameters(params);
}

void SensorTransformationConfig::on_doubleEditorChange(double value)
{
    int index = sender()->property("paramIndex").toInt();
    QVector<QVariant> params = m_transformationList[m_currentIndex]->getParameters();
    params[index] = QVariant(value);
    m_transformationList[m_currentIndex]->setParameters(params);
}

void SensorTransformationConfig::on_booleantEditorChange(int value)
{
    int index = sender()->property("paramIndex").toInt();
    QVector<QVariant> params = m_transformationList[m_currentIndex]->getParameters();
    params[index] = QVariant(value);
    m_transformationList[m_currentIndex]->setParameters(params);
}

void SensorTransformationConfig::on_accept()
{
    m_linkedSensor->setTransformation(configuredTransformation());
    close();
}

void SensorTransformationConfig::on_comboxItemEnter(QModelIndex index)
{
    setToolTipTexts(m_transformationList[index.row()]);
}
