#include <stdio.h>
#include <sqlite3.h>

int main() {
    sqlite3 *db;
    char *errMsg = 0;

    sqlite3_open("test.db", &db);

    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, &errMsg);

    sqlite3_exec(db, "INSERT INTO test VALUES(4,'David');", 0, 0, &errMsg);
    sqlite3_exec(db, "INSERT INTO test VALUES(5,'Eve');", 0, 0, &errMsg);

    sqlite3_exec(db, "COMMIT;", 0, 0, &errMsg);

    printf("Транзакция выполнена\n");

    sqlite3_close(db);
    return 0;
}
