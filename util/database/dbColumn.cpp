#include "dbColumn.h"

DbColumn::DbColumn(QString n, SQLite::DataTypes t)
{
    name = n;
    type = t;
}

QString DbColumn::getName()
{
    return name;
}

SQLite::DataTypes DbColumn::getType()
{
    return type;
}

QString DbColumn::getTypeAsString()
{
    QString str;
    switch (type) {
    case SQLite::INTEGER:
        str = "INTEGER";
        break;
    case SQLite::REAL:
        str = "REAL";
        break;
    case SQLite::TEXT:
        str = "TEXT";
        break;
    default:
        str = "TEXT";
        break;
    }

    return str;
}
