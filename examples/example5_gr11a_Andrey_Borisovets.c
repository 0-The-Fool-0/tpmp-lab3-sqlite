#include <stdio.h>
#include <sqlite3.h>

int main() {
    sqlite3 *db;

    sqlite3_open("test.db", &db);

    printf("SQLite version: %s\n", sqlite3_libversion());

    sqlite3_close(db);
    return 0;
}
