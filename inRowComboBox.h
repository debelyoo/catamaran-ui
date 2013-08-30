#ifndef INROWCOMBOBOX_H
#define INROWCOMBOBOX_H

#include <QComboBox>

class InRowComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit InRowComboBox(QWidget *parent = 0, int ri = -1);
    ~InRowComboBox();

protected:

signals:
    void valueChanged(QString, int);
    void valueChanged(int, int);

public slots:
    void on_activated(QString);
    void on_activated(int);

private:
   int rowIndex;

};

#endif // INROWCOMBOBOX_H
