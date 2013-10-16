#ifndef SENSOR_H
#define SENSOR_H

#include <QObject>
#include "sensorType.h"
#include <QList>
#include "transformation/transformationbaseclass.h"
#include "sensorConfig.h"

class TransformationBaseClass;
class Sensor
{
public:
    Sensor(QString addr = "", QString name = "",
                    const SensorType* type = 0, int display = 0,
                    bool record = false, bool stream = false,
                    QString logFilePrefix = "", QString currentLogFilename = "");
//    explicit Sensor(Sensor *parent, QString addr = "", QString name = "",
//                    SensorType* type = 0, int display = 0,
//                    bool record = false, bool stream = false,
//                    QString logFilePrefix = "", QString currentLogFilename = "");

    ~Sensor();

    // Getters

    int display() const;
    const QString &address() const;
    const QString &name() const;
    const SensorType *type() const;
    bool stream() const;
    bool record() const;
    const TransformationBaseClass *transformation() const;
    const QString &logFilePrefix() const;
    const QString &currentLogFilename() const;

    QString fullname() const;

    // Setters
    void setName(const QString &name);
    void setDisplay(int g);
    void setType(const SensorType *type);
    void setStream(bool enable);
    void setRecord(bool enable);
    void setTransformation(const TransformationBaseClass *tr);
    void setLogFilePrefix(const QString &prefix);
    void setCurrentLogFilename(const QString &fn);

    // Modifiers

protected:
    bool addChild(Sensor *child, bool checkDuplicate = true);

private:

    QString             m_address;
    QString             m_name;

    const SensorType    *m_type;
    bool                m_record;
    bool                m_stream;
    const TransformationBaseClass *m_transformation;

    int                 m_display;

    QString             m_logFilePrefix;
    QString             m_currentLogFilename;
    /*

public:
    explicit Sensor(int addr = 0, QString name = "",
                    SensorType* type = 0, int display = 0,
                    bool record = false, bool stream = false,
                    QString logFilePrefix = "", QString currentLogFilename = "");
    /// getters
    int getAddress();
    QString getName();
    SensorType* getType();
    int getDisplay();
    bool getRecord();
    bool getStream();
    QString getLogFilePrefix();
    QString getCurrentLogFilename();

    /// setters
    void setAddress(int addr);
    void setName(QString name);
    void setType(SensorType* t);
    void setDisplay(int display);
    void setRecord(bool b);
    void setStream(bool b);
    void setLogFilePrefix(QString prefix);
    void setCurrentLogFilename(QString fn);

    private:
        int address;
        QString name;
        SensorType* type;
        int display;
        bool record;
        bool stream;
        QString logFilePrefix;
        QString currentLogFilename;
        */
};

Q_DECLARE_METATYPE(Sensor)
Q_DECLARE_METATYPE(QList<Sensor *>)

#endif // SENSOR_H
