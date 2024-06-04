#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

#define DATABASE_NAME "hagelslag.db"

void create_tables();

sqlite3_stmt* insert_statement(sqlite3* db);

#endif // DATABASE_H
