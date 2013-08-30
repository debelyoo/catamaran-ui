#include "inRowCheckBox.h"

InRowCheckBox::InRowCheckBox(QWidget *parent, int ri) :
    QCheckBox(parent)
{
    rowIndex = ri;
    connect(this, SIGNAL(clicked(bool)), this, SLOT(on_clicked(bool)));
}

InRowCheckBox::~InRowCheckBox()
{

}

void InRowCheckBox::on_clicked(bool b)
{
    emit clicked(b, rowIndex);
}

