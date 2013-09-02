#include "inRowLineEdit.h"

InRowLineEdit::InRowLineEdit(QWidget *parent, int ri) :
    QLineEdit(parent)
{
    rowIndex = ri;
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(on_textChanged(QString)));
}

InRowLineEdit::~InRowLineEdit()
{

}

void InRowLineEdit::on_textChanged(QString text)
{
    emit textChanged(text, rowIndex);
}
