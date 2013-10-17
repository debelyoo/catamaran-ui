#ifndef DBTABLE_H
#define DBTABLE_H

#include "dbColumn.h"

class DbTable
{
    public:
        DbTable(QString name, QList<DbColumn>);
        DbTable();
        QString getName();
        QList<DbColumn> getColumns();

    private:
        QString name;
        QList<DbColumn> columns;
};

#endif // DBTABLE_H
