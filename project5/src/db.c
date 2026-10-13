#include "db.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int exec_simple(sqlite3 *db, const char *sql) {
    char *err = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err ? err : "unknown");
        sqlite3_free(err);
        return rc;
    }
    return SQLITE_OK;
}

int db_open(const char *path, sqlite3 **db) {
    int rc = sqlite3_open(path, db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Не удалось открыть БД: %s\n", sqlite3_errmsg(*db));
        sqlite3_close(*db);
        *db = NULL;
    }
    return rc;
}

void db_close(sqlite3 *db) {
    if (db)
        sqlite3_close(db);
}

int db_init_schema(sqlite3 *db) {
    static const char *sql =
        "PRAGMA foreign_keys = ON;"
        "CREATE TABLE IF NOT EXISTS area ("
        "  id INTEGER PRIMARY KEY,"
        "  area_name TEXT NOT NULL,"
        "  area_description TEXT"
        ");"
        "CREATE TABLE IF NOT EXISTS state ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  country_name TEXT NOT NULL,"
        "  capital TEXT,"
        "  official_language TEXT,"
        "  population INTEGER,"
        "  territory_area REAL,"
        "  currency TEXT,"
        "  government_form TEXT,"
        "  head_of_state TEXT,"
        "  photo BLOB,"
        "  area_id INTEGER,"
        "  FOREIGN KEY(area_id) REFERENCES area(id)"
        ");";

    return exec_simple(db, sql);
}

int db_seed_demo(sqlite3 *db) {
    sqlite3_stmt *st = NULL;
    int rc = sqlite3_prepare_v2(db,
                                "SELECT COUNT(*) FROM area;", -1, &st, NULL);
    if (rc != SQLITE_OK)
        return rc;
    if (sqlite3_step(st) != SQLITE_ROW) {
        sqlite3_finalize(st);
        return SQLITE_ERROR;
    }
    int n = sqlite3_column_int(st, 0);
    sqlite3_finalize(st);
    if (n > 0)
        return SQLITE_OK;

    rc = exec_simple(db,
                     "INSERT INTO area (id, area_name, area_description) VALUES "
                     "(1, 'Европа', 'Европейский регион'),"
                     "(2, 'Азия', 'Азиатский регион'),"
                     "(3, 'Африка', 'Африканский регион');");
    if (rc != SQLITE_OK)
        return rc;

    static const char *ins =
        "INSERT INTO state (country_name, capital, official_language, population, "
        "territory_area, currency, government_form, head_of_state, area_id, photo) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    rc = sqlite3_prepare_v2(db, ins, -1, &st, NULL);
    if (rc != SQLITE_OK)
        return rc;

    const unsigned char demo_png[] = {
        0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
    struct {
        const char *c, *cap, *lang;
        sqlite3_int64 pop;
        double area;
        const char *cur, *gov, *head;
        int aid;
    } rows[] = {
        {"Беларусь", "Минск", "белорусский", 9200000, 207600.0, "BYN",
         "республика", "Президент", 1},
        {"Польша", "Варшава", "польский", 38000000, 312696.0, "PLN",
         "республика", "Президент", 1},
        {"Япония", "Токио", "японский", 125000000, 377975.0, "JPY",
         "конституционная монархия", "Император", 2},
    };

    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        sqlite3_reset(st);
        sqlite3_bind_text(st, 1, rows[i].c, -1, SQLITE_STATIC);
        sqlite3_bind_text(st, 2, rows[i].cap, -1, SQLITE_STATIC);
        sqlite3_bind_text(st, 3, rows[i].lang, -1, SQLITE_STATIC);
        sqlite3_bind_int64(st, 4, rows[i].pop);
        sqlite3_bind_double(st, 5, rows[i].area);
        sqlite3_bind_text(st, 6, rows[i].cur, -1, SQLITE_STATIC);
        sqlite3_bind_text(st, 7, rows[i].gov, -1, SQLITE_STATIC);
        sqlite3_bind_text(st, 8, rows[i].head, -1, SQLITE_STATIC);
        sqlite3_bind_int(st, 9, rows[i].aid);
        if (i == 0)
            sqlite3_bind_blob(st, 10, demo_png, (int)sizeof(demo_png),
                              SQLITE_TRANSIENT);
        else
            sqlite3_bind_null(st, 10);
        rc = sqlite3_step(st);
        if (rc != SQLITE_DONE) {
            sqlite3_finalize(st);
            return rc;
        }
    }
    sqlite3_finalize(st);
    return SQLITE_OK;
}

void db_print_state_row(int id, const char *country, const char *capital,
                        const char *language, sqlite3_int64 population,
                        double territory, const char *currency,
                        const char *gov_form, const char *head, int area_id,
                        const char *area_name, int has_photo) {
    printf(
        "id=%d | %s | столица: %s | язык: %s | население: %lld | площадь: %.1f "
        "км² | валюта: %s | строй: %s | глава: %s | часть света: %s (area_id=%d) "
        "| фото: %s\n",
        id, country, capital, language, (long long)population, territory,
        currency, gov_form, head, area_name ? area_name : "—",
        area_id > 0 ? area_id : 0, has_photo ? "да" : "нет");
}

static void print_from_stmt(sqlite3_stmt *st) {
    int id = sqlite3_column_int(st, 0);
    const char *country = (const char *)sqlite3_column_text(st, 1);
    const char *capital = (const char *)sqlite3_column_text(st, 2);
    const char *lang = (const char *)sqlite3_column_text(st, 3);
    sqlite3_int64 pop = sqlite3_column_int64(st, 4);
    double terr = sqlite3_column_double(st, 5);
    const char *cur = (const char *)sqlite3_column_text(st, 6);
    const char *gov = (const char *)sqlite3_column_text(st, 7);
    const char *head = (const char *)sqlite3_column_text(st, 8);
    int area_id = sqlite3_column_type(st, 9) == SQLITE_NULL
                      ? 0
                      : sqlite3_column_int(st, 9);
    const char *aname = sqlite3_column_type(st, 10) == SQLITE_NULL
                            ? NULL
                            : (const char *)sqlite3_column_text(st, 10);
    int has_photo = sqlite3_column_type(st, 11) != SQLITE_NULL;
    db_print_state_row(id, country ? country : "", capital ? capital : "",
                       lang ? lang : "", pop, terr, cur ? cur : "",
                       gov ? gov : "", head ? head : "", area_id, aname,
                       has_photo);
}

int db_list_all(sqlite3 *db) {
    static const char *sql =
        "SELECT s.id, s.country_name, s.capital, s.official_language, "
        "s.population, s.territory_area, s.currency, s.government_form, "
        "s.head_of_state, s.area_id, a.area_name, s.photo "
        "FROM state s LEFT JOIN area a ON s.area_id = a.id "
        "ORDER BY s.id;";
    sqlite3_stmt *st = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK)
        return rc;
    while ((rc = sqlite3_step(st)) == SQLITE_ROW)
        print_from_stmt(st);
    sqlite3_finalize(st);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

int db_select_by_id(sqlite3 *db, int id) {
    static const char *sql =
        "SELECT s.id, s.country_name, s.capital, s.official_language, "
        "s.population, s.territory_area, s.currency, s.government_form, "
        "s.head_of_state, s.area_id, a.area_name, s.photo "
        "FROM state s LEFT JOIN area a ON s.area_id = a.id "
        "WHERE s.id = ?;";
    sqlite3_stmt *st = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK)
        return rc;
    sqlite3_bind_int(st, 1, id);
    rc = sqlite3_step(st);
    if (rc == SQLITE_ROW) {
        print_from_stmt(st);
        rc = SQLITE_OK;
    } else if (rc == SQLITE_DONE) {
        printf("Запись с id=%d не найдена.\n", id);
        rc = SQLITE_OK;
    }
    sqlite3_finalize(st);
    return rc;
}

int db_select_by_country_pattern(sqlite3 *db, const char *pattern) {
    static const char *sql =
        "SELECT s.id, s.country_name, s.capital, s.official_language, "
        "s.population, s.territory_area, s.currency, s.government_form, "
        "s.head_of_state, s.area_id, a.area_name, s.photo "
        "FROM state s LEFT JOIN area a ON s.area_id = a.id "
        "WHERE s.country_name LIKE ? ESCAPE '\\' "
        "ORDER BY s.country_name;";
    sqlite3_stmt *st = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK)
        return rc;
    char buf[256];
    snprintf(buf, sizeof(buf), "%%%s%%", pattern);
    sqlite3_bind_text(st, 1, buf, -1, SQLITE_TRANSIENT);
    int found = 0;
    while ((rc = sqlite3_step(st)) == SQLITE_ROW) {
        print_from_stmt(st);
        found = 1;
    }
    sqlite3_finalize(st);
    if (!found)
        printf("Нет стран, содержащих \"%s\" в названии.\n", pattern);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

int db_select_by_capital_pattern(sqlite3 *db, const char *pattern) {
    static const char *sql =
        "SELECT s.id, s.country_name, s.capital, s.official_language, "
        "s.population, s.territory_area, s.currency, s.government_form, "
        "s.head_of_state, s.area_id, a.area_name, s.photo "
        "FROM state s LEFT JOIN area a ON s.area_id = a.id "
        "WHERE s.capital LIKE ? ESCAPE '\\' "
        "ORDER BY s.capital;";
    sqlite3_stmt *st = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK)
        return rc;
    char buf[256];
    snprintf(buf, sizeof(buf), "%%%s%%", pattern);
    sqlite3_bind_text(st, 1, buf, -1, SQLITE_TRANSIENT);
    int found = 0;
    while ((rc = sqlite3_step(st)) == SQLITE_ROW) {
        print_from_stmt(st);
        found = 1;
    }
    sqlite3_finalize(st);
    if (!found)
        printf("Нет записей со столицей, содержащей \"%s\".\n", pattern);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

int db_select_by_area_id(sqlite3 *db, int area_id) {
    static const char *sql =
        "SELECT s.id, s.country_name, s.capital, s.official_language, "
        "s.population, s.territory_area, s.currency, s.government_form, "
        "s.head_of_state, s.area_id, a.area_name, s.photo "
        "FROM state s LEFT JOIN area a ON s.area_id = a.id "
        "WHERE s.area_id = ? "
        "ORDER BY s.country_name;";
    sqlite3_stmt *st = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK)
        return rc;
    sqlite3_bind_int(st, 1, area_id);
    int found = 0;
    while ((rc = sqlite3_step(st)) == SQLITE_ROW) {
        print_from_stmt(st);
        found = 1;
    }
    sqlite3_finalize(st);
    if (!found)
        printf("Нет государств с area_id=%d.\n", area_id);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

int db_list_areas(sqlite3 *db) {
    static const char *sql =
        "SELECT id, area_name, area_description FROM area ORDER BY id;";
    sqlite3_stmt *st = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK)
        return rc;
    printf("Части света (area):\n");
    while ((rc = sqlite3_step(st)) == SQLITE_ROW) {
        printf("  id=%d | %s | %s\n", sqlite3_column_int(st, 0),
               sqlite3_column_text(st, 1),
               sqlite3_column_text(st, 2) ? (const char *)sqlite3_column_text(
                                                  st, 2)
                                            : "");
    }
    sqlite3_finalize(st);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

int db_insert_state(sqlite3 *db, const char *country, const char *capital,
                    const char *language, sqlite3_int64 population,
                    double territory, const char *currency,
                    const char *gov_form, const char *head, int area_id,
                    const unsigned char *photo, int photo_len) {
    static const char *sql =
        "INSERT INTO state (country_name, capital, official_language, population, "
        "territory_area, currency, government_form, head_of_state, area_id, photo) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *st = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK)
        return rc;
    sqlite3_bind_text(st, 1, country, -1, SQLITE_STATIC);
    sqlite3_bind_text(st, 2, capital, -1, SQLITE_STATIC);
    sqlite3_bind_text(st, 3, language, -1, SQLITE_STATIC);
    sqlite3_bind_int64(st, 4, population);
    sqlite3_bind_double(st, 5, territory);
    sqlite3_bind_text(st, 6, currency, -1, SQLITE_STATIC);
    sqlite3_bind_text(st, 7, gov_form, -1, SQLITE_STATIC);
    sqlite3_bind_text(st, 8, head, -1, SQLITE_STATIC);
    if (area_id > 0)
        sqlite3_bind_int(st, 9, area_id);
    else
        sqlite3_bind_null(st, 9);
    if (photo && photo_len > 0)
        sqlite3_bind_blob(st, 10, photo, photo_len, SQLITE_TRANSIENT);
    else
        sqlite3_bind_null(st, 10);
    rc = sqlite3_step(st);
    sqlite3_finalize(st);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

int db_delete_by_id(sqlite3 *db, int id) {
    static const char *sql = "DELETE FROM state WHERE id = ?;";
    sqlite3_stmt *st = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK)
        return rc;
    sqlite3_bind_int(st, 1, id);
    rc = sqlite3_step(st);
    sqlite3_finalize(st);
    if (rc == SQLITE_DONE) {
        if (sqlite3_changes(db) == 0) {
            printf("Удаление: id=%d не найден.\n", id);
        } else {
            printf("Удалена запись id=%d.\n", id);
        }
        return SQLITE_OK;
    }
    return rc;
}

int db_export_photo(sqlite3 *db, int id, const char *filepath) {
    static const char *sql = "SELECT photo FROM state WHERE id = ?;";
    sqlite3_stmt *st = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc != SQLITE_OK)
        return rc;
    sqlite3_bind_int(st, 1, id);
    rc = sqlite3_step(st);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(st);
        printf("Нет записи или нет фото для id=%d.\n", id);
        return SQLITE_OK;
    }
    if (sqlite3_column_type(st, 0) == SQLITE_NULL) {
        sqlite3_finalize(st);
        printf("У записи id=%d поле фото пустое.\n", id);
        return SQLITE_OK;
    }
    int len = sqlite3_column_bytes(st, 0);
    const void *blob = sqlite3_column_blob(st, 0);
    unsigned char *copy = malloc((size_t)len);
    if (!copy) {
        sqlite3_finalize(st);
        return SQLITE_NOMEM;
    }
    memcpy(copy, blob, (size_t)len);
    sqlite3_finalize(st);

    FILE *f = fopen(filepath, "wb");
    if (!f) {
        free(copy);
        perror("fopen");
        return SQLITE_ERROR;
    }
    if (fwrite(copy, 1, (size_t)len, f) != (size_t)len) {
        free(copy);
        fclose(f);
        fprintf(stderr, "Ошибка записи файла.\n");
        return SQLITE_ERROR;
    }
    free(copy);
    fclose(f);
    printf("Фото записано в \"%s\" (%d байт).\n", filepath, len);
    return SQLITE_OK;
}

int db_demo_autocommit_inserts(sqlite3 *db) {
    printf("--- Демо: три INSERT подряд в режиме autocommit (отдельные "
           "транзакции) ---\n");
    static const char *rows[][2] = {{"АвтоA1", "C1"}, {"АвтоA2", "C2"},
                                      {"АвтоA3", "C3"}};
    for (int i = 0; i < 3; i++) {
        if (db_insert_state(db, rows[i][0], rows[i][1], "n/a", 1000000,
                            10000.0, "XXX", "демо", "демо", 1, NULL, 0) !=
            SQLITE_OK) {
            fprintf(stderr, "Ошибка autocommit INSERT #%d\n", i + 1);
            return SQLITE_ERROR;
        }
        printf("INSERT #%d зафиксирован автоматически.\n", i + 1);
    }
    printf("Готово.\n");
    return SQLITE_OK;
}

int db_demo_transaction_inserts(sqlite3 *db) {
    printf("--- Демо: три INSERT в одной явной транзакции BEGIN/COMMIT ---\n");
    int rc = exec_simple(db, "BEGIN IMMEDIATE;");
    if (rc != SQLITE_OK)
        return rc;
    static const char *rows[][2] = {{"ТранT1", "TC1"}, {"ТранT2", "TC2"},
                                    {"ТранT3", "TC3"}};
    for (int i = 0; i < 3; i++) {
        rc = db_insert_state(db, rows[i][0], rows[i][1], "n/a", 2000000,
                             20000.0, "YYY", "демо", "демо", 2, NULL, 0);
        if (rc != SQLITE_OK) {
            exec_simple(db, "ROLLBACK;");
            return rc;
        }
        printf("INSERT #%d внутри транзакции (ещё не виден другим).\n", i + 1);
    }
    rc = exec_simple(db, "COMMIT;");
    if (rc == SQLITE_OK)
        printf("COMMIT: все три строки зафиксированы разом.\n");
    return rc;
}
