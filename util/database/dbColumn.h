#ifndef DBCOLUMN_H
#define DBCOLUMN_H

#include <QString>
#include <QList>

namespace SQLite {
    typedef enum {
        INTEGER = 0,
        REAL = 1,
        TEXT = 2,
        BLOB = 3
    } DataTypes;
}

class DbColumn
{
    public:
        DbColumn(QString name, SQLite::DataTypes type);
        QString getName();
        SQLite::DataTypes getType();
        QString getTypeAsString();

    private:
        QString name;
        SQLite::DataTypes type;
};

#endif // DBCOLUMN_H
