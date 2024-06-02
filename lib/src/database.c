#include <stdlib.h>

#include "database.h"
#include "logger.h"

void create_tables() {
  sqlite3* db;

  int result =
      sqlite3_open_v2(DATABASE_NAME, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

  if (result != SQLITE_OK) {
    FATAL("DATABASE", "Can't open database connection: %s", sqlite3_errmsg(db));
  }

  // Create the 'connection' and 'get' tables.
  const char* sql = "CREATE TABLE IF NOT EXISTS connection ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "address INTEGER"
                    ");"
                    "CREATE TABLE IF NOT EXISTS get ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "address INTEGER"
                    ");";

  result = sqlite3_exec(db, sql, 0, 0, 0);
  if (result != SQLITE_OK) {
    FATAL("DATABASE", "Can't create table: %s", sqlite3_errmsg(db));
  }

  sqlite3_close(db);
}

// Statement to insert an address into the 'connection' table.
sqlite3_stmt* insert_conn_statement(sqlite3* db) {
  sqlite3_stmt* conn_stmt;

  const char* sql = "INSERT INTO connection (address) VALUES (?);";

  int result = sqlite3_prepare_v3(db, sql, -1, SQLITE_PREPARE_PERSISTENT, &conn_stmt, NULL);
  if (result != SQLITE_OK) {
    FATAL("DATABASE", "Can't prepare statement: %s", sqlite3_errmsg(db));
  }

  return conn_stmt;
}

// Statement to insert an address into the 'get' table.
sqlite3_stmt* insert_get_statement(sqlite3* db) {
  sqlite3_stmt* get_stmt;

  const char* sql = "INSERT INTO get (address) VALUES (?);";

  int result = sqlite3_prepare_v3(db, sql, -1, SQLITE_PREPARE_PERSISTENT, &get_stmt, NULL);
  if (result != SQLITE_OK) {
    FATAL("DATABASE", "Can't prepare statement: %s", sqlite3_errmsg(db));
  }

  return get_stmt;
}
