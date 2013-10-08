#include "registeredSensorsDelegate.h"
#include <QPainter>
#include <QDebug>

RegisteredSensorsDelegate::RegisteredSensorsDelegate(int column, QObject *parent) :
    QStyledItemDelegate(parent),
    m_column(column)
{
    m_tableView = qobject_cast<QTableView *>(parent);
    m_button = new QPushButton("...", m_tableView);
    m_button->hide();
    m_button->setFlat(true);
    m_button->setStyleSheet( "background-color: transparent; color: blue; text-decoration: underline;" );
    m_tableView->setMouseTracking(true);
    connect(m_tableView, SIGNAL(entered(QModelIndex)),
            this, SLOT(cellEntered(QModelIndex)));
    m_isOneCellInEditMode = false;
}

void RegisteredSensorsDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //if( index.model()->headerData(index.column(), Qt::Horizontal, Qt::UserRole).toInt() == 1 )
    if(!(m_isOneCellInEditMode && m_currentEditedCellIndex == index))
    {
        m_button->setGeometry(option.rect);
        m_button->setText(index.data().toString());
        //if (option.state == QStyle::State_Selected)
        //    painter->fillRect(option.rect, option.palette.highlight());
        //qDebug() << "state : " << option.state;
        QPixmap map = m_button->grab();
        painter->drawPixmap(option.rect.x(),option.rect.y(),map);
    } else {
        //QStyledItemDelegate::paint(painter,option, index);
    }
}

QWidget *RegisteredSensorsDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    QPushButton *bt = new QPushButton("", parent);
    bt->setFlat(true);
    bt->setStyleSheet( "background-color: transparent; color: blue; text-decoration: none; font-weight: bold;" );
    bt->setProperty("index", QVariant(index));
    connect(bt, SIGNAL(clicked()), this, SLOT(on_buttonClicked()));
    return bt;
}

void RegisteredSensorsDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QPushButton *bt = static_cast<QPushButton*>(editor);
    bt->setText(index.model()->data(index, Qt::EditRole).toString());
}

void RegisteredSensorsDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

void RegisteredSensorsDelegate::cellEntered(const QModelIndex &index)
{
    if(index.column() == m_column)
    {
        if(m_isOneCellInEditMode)
        {
            m_tableView->closePersistentEditor(m_currentEditedCellIndex);
        }
        m_tableView->openPersistentEditor(index);
        m_isOneCellInEditMode = true;
        m_currentEditedCellIndex = index;
    } else {
        if(m_isOneCellInEditMode)
        {
            m_isOneCellInEditMode = false;
            m_tableView->closePersistentEditor(m_currentEditedCellIndex);
        }
    }
}

void RegisteredSensorsDelegate::on_buttonClicked()
{
    QPushButton *bt = static_cast<QPushButton *>(sender());
    //qDebug() << "Delegate : bt click " << bt->property("index").toModelIndex();
    QModelIndex index = bt->property("index").toModelIndex();
    emit buttonClicked(index);
}
