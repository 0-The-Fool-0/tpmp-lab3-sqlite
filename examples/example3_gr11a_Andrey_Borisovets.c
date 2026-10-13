#include <stdio.h>
#include <sqlite3.h>

int main() {
    sqlite3 *db;
    sqlite3_stmt *stmt;

    sqlite3_open("test.db", &db);

    const char *sql = "INSERT INTO test (ID, NAME) VALUES (?, ?);";

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    sqlite3_bind_int(stmt, 1, 3);
    sqlite3_bind_text(stmt, 2, "Charlie", -1, SQLITE_STATIC);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    printf("Данные вставлены\n");

    sqlite3_close(db);
    return 0;
}
