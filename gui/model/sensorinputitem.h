#ifndef SENSORINPUTITEM_H
#define SENSORINPUTITEM_H

#include <QList>
#include <QString>
#include <QObject>

class SensorInputItem : public QObject
{
    Q_OBJECT

public:
    SensorInputItem(const QString &address, const QString &name, SensorInputItem *pParent = NULL);
    SensorInputItem(const QString &name, SensorInputItem *pParent = NULL);
    ~SensorInputItem();
    SensorInputItem *parent() const;
    void setParent(SensorInputItem *pParent);

    QList<SensorInputItem *> childs() const;
    void setChilds(const QList<SensorInputItem *> &childs);
    void addChild(SensorInputItem *child);
    void insertChild(SensorInputItem *child, int pos);
    bool removeChild(int index);
    void deleteChilds();
    SensorInputItem *child(int index) const;

    QString name() const;
    QString fullName() const;
    const QString address() const;
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
    QList<SensorInputItem *> m_childs;
    QString m_name;
    QString m_address;
    bool m_enabled;
};

#endif // SENSORINPUTITEM_H
