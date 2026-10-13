#ifndef DB_H
#define DB_H

#include <sqlite3.h>
#include <stddef.h>

int db_open(const char *path, sqlite3 **db);
void db_close(sqlite3 *db);
int db_init_schema(sqlite3 *db);
int db_seed_demo(sqlite3 *db);

void db_print_state_row(int id, const char *country, const char *capital,
                        const char *language, sqlite3_int64 population,
                        double territory, const char *currency,
                        const char *gov_form, const char *head,
                        int area_id, const char *area_name, int has_photo);

int db_list_all(sqlite3 *db);
int db_select_by_id(sqlite3 *db, int id);
int db_select_by_country_pattern(sqlite3 *db, const char *pattern);
int db_select_by_capital_pattern(sqlite3 *db, const char *pattern);
int db_select_by_area_id(sqlite3 *db, int area_id);
int db_list_areas(sqlite3 *db);

int db_insert_state(sqlite3 *db, const char *country, const char *capital,
                    const char *language, sqlite3_int64 population,
                    double territory, const char *currency,
                    const char *gov_form, const char *head, int area_id,
                    const unsigned char *photo, int photo_len);

int db_delete_by_id(sqlite3 *db, int id);
int db_export_photo(sqlite3 *db, int id, const char *filepath);

int db_demo_autocommit_inserts(sqlite3 *db);
int db_demo_transaction_inserts(sqlite3 *db);

#endif
