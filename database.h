#include <stdint.h>

#include <sqlite3.h>

#define DATABASE_NAME "hagelslag.db"

void create_tables();

sqlite3_stmt *insert_conn_statement(sqlite3 *db);
sqlite3_stmt *insert_get_statement(sqlite3 *db);

int exists(sqlite3_stmt *statement, const char *address);
