#ifndef INROWLINEEDIT_H
#define INROWLINEEDIT_H

#include <QLineEdit>

class InRowLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit InRowLineEdit(QWidget *parent = 0, int ri = -1);
    ~InRowLineEdit();

protected:

signals:
    void textChanged(QString, int);

public slots:
    void on_textChanged(QString);

private:
   int rowIndex;

};

#endif // INROWLINEEDIT_H
