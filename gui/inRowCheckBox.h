#ifndef INROWCHECKBOX_H
#define INROWCHECKBOX_H

#include <QCheckBox>

class InRowCheckBox : public QCheckBox
{
    Q_OBJECT
public:
    explicit InRowCheckBox(QWidget *parent = 0, int ri = -1);
    ~InRowCheckBox();

protected:

signals:
    void clicked(bool, int);

public slots:
    void on_clicked(bool);

private:
   int rowIndex;

};

#endif // INROWCHECKBOX_H
