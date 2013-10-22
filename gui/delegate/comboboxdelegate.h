#ifndef COMBOBOXDELEGATE_H
#define COMBOBOXDELEGATE_H

#include <QStyledItemDelegate>
#include <QTableView>
#include <QComboBox>

class ComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ComboBoxDelegate(int column, QObject *parent = 0);
    QWidget *createEditor(QWidget *parent,
                                            const QStyleOptionViewItem &/* option */,
                                            const QModelIndex &/* index */) const;
    void setEditorData(QWidget *editor,
                                         const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                                        const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor,
                                                const QStyleOptionViewItem &option, const QModelIndex &/* index */) const;

signals:
    //void comboChange(const QModelIndex &index,const QModelIndex &index);

public slots:

    void cellEntered(const QModelIndex &index);

protected:
    QComboBox *m_button;
    QTableView *m_tableView;

    bool m_isOneCellInEditMode;
    QPersistentModelIndex m_currentEditedCellIndex;
    int m_column;

protected slots:
    void on_comboChange();
};

#endif // COMBOBOXDELEGATE_H
