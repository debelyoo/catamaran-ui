#ifndef SENSORINPUTITEM_H
#define SENSORINPUTITEM_H

#include <QVector>
#include <QString>
#include <QObject>

class SensorInputItem : public QObject
{
    Q_OBJECT

public:
    SensorInputItem(QString name = "", SensorInputItem *pParent = NULL);
    SensorInputItem *parent() const;
    void setParent(SensorInputItem *pParent);

    QVector<SensorInputItem *> childs() const;
    void setChilds(const QVector<SensorInputItem *> &childs);
    void addChild(SensorInputItem *child);
    void insertChild(SensorInputItem *child, int pos);
    SensorInputItem *removeChild(int index);
    SensorInputItem *child(int index) const;

    QString name() const;
    QString fullName() const;
    void setName(QString name);


    int childCount() const;
    int childNumber() const;

    QString sortId() const;

    void enable();
    void disable();
    bool enabled() const;
signals:
    void itemChanged();

protected:
    SensorInputItem *m_pParent;
    QVector<SensorInputItem *> m_childs;
    QString m_name;
    bool m_enabled;
};

#endif // SENSORINPUTITEM_H
