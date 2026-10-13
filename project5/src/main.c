#include "db.h"
#include "menu.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    const char *dbpath = "states_lab3.db";
    if (argc > 1)
        dbpath = argv[1];

    sqlite3 *db = NULL;
    if (db_open(dbpath, &db) != SQLITE_OK)
        return 1;

    if (db_init_schema(db) != SQLITE_OK) {
        db_close(db);
        return 1;
    }
    if (db_seed_demo(db) != SQLITE_OK) {
        fprintf(stderr, "Предупреждение: не удалось заполнить демо-данные.\n");
    }

    printf("БД: %s\n", dbpath);
    menu_run(db);
    db_close(db);
    return 0;
}
