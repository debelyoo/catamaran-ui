#include "dbTable.h"

DbTable::DbTable(QString n, QList<DbColumn> cols)
{
    name = n;
    columns = cols;
}

/**
 * no-parameter constructor
 * @brief DbTable::DbTable
 */
DbTable::DbTable()
{
    name = "bulk";
    columns = QList<DbColumn>();
}

QString DbTable::getName()
{
    return name;
}

QList<DbColumn> DbTable::getColumns()
{
    return columns;
}

