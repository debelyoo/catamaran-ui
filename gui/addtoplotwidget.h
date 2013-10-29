#ifndef ADDTOPLOTWIDGET_H
#define ADDTOPLOTWIDGET_H

#include <QWidget>
#include <QList>

namespace Ui {
class AddToPlotWidget;
}

class AddToPlotWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AddToPlotWidget(int plotNumber, QWidget *parent = 0);
    ~AddToPlotWidget();


signals:
    void savedButtonClicked(bool configHasChanged);
    void cancelButtonClicked();
    void resetButtonClicked();

private:
    Ui::AddToPlotWidget *ui;
    int m_plotNumber;

    void fillSensorList();

private slots:
    void on_save();
    void on_reset();
};

#endif // ADDTOPLOTWIDGET_H
