#ifndef REGISTEREDSENSORSDELEGATE_H
#define REGISTEREDSENSORSDELEGATE_H

#include <QStyledItemDelegate>
#include <QPushButton>
#include <QTableView>

class RegisteredSensorsDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit RegisteredSensorsDelegate(int column, QObject *parent = 0);
signals:
    void buttonClicked(QModelIndex &index);

public slots:
    // QAbstractItemDelegate interface
    void cellEntered(const QModelIndex &index);

    void on_buttonClicked();
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
    QPushButton *m_button;
    QTableView *m_tableView;
    bool m_isOneCellInEditMode;
    QPersistentModelIndex m_currentEditedCellIndex;

    int m_column;

    // QAbstractItemDelegate interface
public:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // REGISTEREDSENSORSDELEGATE_H
