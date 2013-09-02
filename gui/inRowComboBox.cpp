#include "inRowComboBox.h"

InRowComboBox::InRowComboBox(QWidget *parent, int ri) :
    QComboBox(parent)
{
    rowIndex = ri;
    connect(this, SIGNAL(activated(QString)), this, SLOT(on_activated(QString)));
    connect(this, SIGNAL(activated(int)), this, SLOT(on_activated(int)));
}

InRowComboBox::~InRowComboBox()
{

}

void InRowComboBox::on_activated(QString text)
{
    emit valueChanged(text, rowIndex);
}

void InRowComboBox::on_activated(int ind)
{
    emit valueChanged(ind, rowIndex);
}

