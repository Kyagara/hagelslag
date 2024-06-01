#include <sqlite3.h>

int create_socket();
void scan(sqlite3 *db, sqlite3_stmt *conn_stmt, sqlite3_stmt *get_stmt, int socket_fd,
          const char *address);
