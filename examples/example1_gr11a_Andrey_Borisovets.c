#include <stdio.h>
#include <sqlite3.h>

int main() {
    sqlite3 *db;
    int rc;

    rc = sqlite3_open("test.db", &db);

    if (rc) {
        printf("Ошибка открытия БД\n");
        return 1;
    } else {
        printf("БД успешно открыта\n");
    }

    sqlite3_close(db);
    return 0;
}
