#include <stdio.h>
#include <sqlite3.h>

static int callback(void *data, int argc, char **argv, char **azColName){
    for(int i=0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int main() {
    sqlite3 *db;
    char *errMsg = 0;

    sqlite3_open("test.db", &db);

    char *sql =
        "CREATE TABLE IF NOT EXISTS test("
        "ID INT PRIMARY KEY NOT NULL,"
        "NAME TEXT NOT NULL);"

        "INSERT INTO test VALUES(1,'Alex');"
        "INSERT INTO test VALUES(2,'Bob');"

        "SELECT * FROM test;";

    sqlite3_exec(db, sql, callback, 0, &errMsg);

    sqlite3_close(db);
    return 0;
}
