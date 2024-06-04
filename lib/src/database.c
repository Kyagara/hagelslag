#include <stdlib.h>

#include "database.h"
#include "logger.h"

void create_tables() {
  sqlite3* db;

  int result = sqlite3_open_v2(DATABASE_URI, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

  if (result != SQLITE_OK) {
    FATAL("DATABASE", "Can't open database connection: %s", sqlite3_errmsg(db));
  }

  // Create a 'http' table.
  const char* sql = "CREATE TABLE IF NOT EXISTS http ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "address INTEGER"
                    ");";

  result = sqlite3_exec(db, sql, 0, 0, 0);
  if (result != SQLITE_OK) {
    FATAL("DATABASE", "Can't create table: %s", sqlite3_errmsg(db));
  }

  sqlite3_close(db);
}

// Statement to insert an address into the table.
sqlite3_stmt* insert_statement(sqlite3* db) {
  sqlite3_stmt* conn_stmt;

  const char* sql = "INSERT INTO http (address) VALUES (?);";

  int result = sqlite3_prepare_v3(db, sql, -1, SQLITE_PREPARE_PERSISTENT, &conn_stmt, NULL);
  if (result != SQLITE_OK) {
    FATAL("DATABASE", "Can't prepare statement: %s", sqlite3_errmsg(db));
  }

  return conn_stmt;
}
