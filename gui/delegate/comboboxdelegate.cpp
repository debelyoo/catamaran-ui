#include "comboboxdelegate.h"
#include <QComboBox>
#include <QStringListModel>
#include "manager/sensortypemanager.h"

#include <QDebug>

ComboBoxDelegate::ComboBoxDelegate(int column, QObject *parent) :
    QStyledItemDelegate(parent),
    m_column(column)
{
    m_tableView = qobject_cast<QTableView *>(parent);
    m_tableView->setMouseTracking(true);
    connect(m_tableView, SIGNAL(entered(QModelIndex)),
            this, SLOT(cellEntered(QModelIndex)));
    m_isOneCellInEditMode = false;
}

QWidget *ComboBoxDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &/* option */,
                                        const QModelIndex &index) const
{
    Q_UNUSED(index);
    QComboBox *editor = new QComboBox(parent);
    return editor;
}

void ComboBoxDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const
{
    // QString value = index.model()->data(index, Qt::EditRole).toString();

    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    comboBox->disconnect(this);
    comboBox->addItems(SensorTypeManager::instance()->sortedList());
    comboBox->setCurrentText(index.data(Qt::DisplayRole).toString());
    connect(editor, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboChange()));
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentText(), Qt::EditRole);
}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor,
                                            const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

void ComboBoxDelegate::cellEntered(const QModelIndex &index)
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

void ComboBoxDelegate::on_comboChange()
{
    if(m_isOneCellInEditMode){
        setModelData((QComboBox *)sender(), m_tableView->model(), m_currentEditedCellIndex);
    }
}
