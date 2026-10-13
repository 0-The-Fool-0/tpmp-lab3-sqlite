#include "menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void trim_newline(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = '\0';
        n--;
    }
}

static void read_line(const char *prompt, char *buf, size_t cap) {
    printf("%s", prompt);
    if (!fgets(buf, (int)cap, stdin))
        buf[0] = '\0';
    trim_newline(buf);
}

static long read_long(const char *prompt, long def) {
    char buf[64];
    read_line(prompt, buf, sizeof(buf));
    if (buf[0] == '\0')
        return def;
    char *end = NULL;
    long v = strtol(buf, &end, 10);
    if (end == buf)
        return def;
    return v;
}

static double read_double(const char *prompt, double def) {
    char buf[64];
    read_line(prompt, buf, sizeof(buf));
    if (buf[0] == '\0')
        return def;
    char *end = NULL;
    double v = strtod(buf, &end);
    if (end == buf)
        return def;
    return v;
}

static int read_file_blob(const char *path, unsigned char **out, int *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        perror(path);
        return -1;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return -1;
    }
    long sz = ftell(f);
    if (sz < 0 || sz > 16 * 1024 * 1024) {
        fclose(f);
        fprintf(stderr, "Недопустимый размер файла.\n");
        return -1;
    }
    rewind(f);
    unsigned char *b = malloc((size_t)sz);
    if (!b) {
        fclose(f);
        return -1;
    }
    if (fread(b, 1, (size_t)sz, f) != (size_t)sz) {
        free(b);
        fclose(f);
        return -1;
    }
    fclose(f);
    *out = b;
    *out_len = (int)sz;
    return 0;
}

void menu_run(sqlite3 *db) {
    for (;;) {
        printf(
            "\n=== Лаб.3, задание 5, вариант 3 (государства / части света) ===\n"
            "1 — показать все государства\n"
            "2 — выборка по id (параметризованный SELECT)\n"
            "3 — выборка по фрагменту названия страны (LIKE)\n"
            "4 — выборка по фрагменту столицы (LIKE)\n"
            "5 — выборка по части света (area_id, общее поле)\n"
            "6 — список частей света (area)\n"
            "7 — вставить государство (INSERT + опционально фото из файла)\n"
            "8 — удалить государство по id (DELETE)\n"
            "9 — экспорт фото в файл\n"
            "10 — демо: autocommit (три отдельных INSERT)\n"
            "11 — демо: одна транзакция (BEGIN…COMMIT, три INSERT)\n"
            "0 — выход\n"
            "Выбор: ");
        char line[16];
        if (!fgets(line, sizeof(line), stdin))
            break;
        int choice = atoi(line);

        char b1[256], b2[256], b3[256], b4[64], b5[128], b6[128], b7[128];
        long pop;
        double terr;
        int aid;

        switch (choice) {
        case 0:
            return;
        case 1:
            db_list_all(db);
            break;
        case 2: {
            int id = (int)read_long("id: ", -1);
            if (id < 0)
                printf("Некорректный id.\n");
            else
                db_select_by_id(db, id);
            break;
        }
        case 3:
            read_line("Фрагмент названия страны: ", b1, sizeof(b1));
            if (b1[0])
                db_select_by_country_pattern(db, b1);
            break;
        case 4:
            read_line("Фрагмент столицы: ", b1, sizeof(b1));
            if (b1[0])
                db_select_by_capital_pattern(db, b1);
            break;
        case 5:
            db_list_areas(db);
            aid = (int)read_long("area_id: ", 0);
            if (aid > 0)
                db_select_by_area_id(db, aid);
            break;
        case 6:
            db_list_areas(db);
            break;
        case 7:
            read_line("Страна: ", b1, sizeof(b1));
            read_line("Столица: ", b2, sizeof(b2));
            read_line("Гос. язык: ", b3, sizeof(b3));
            pop = read_long("Население: ", 0);
            terr = read_double("Площадь (км²): ", 0);
            read_line("Валюта: ", b4, sizeof(b4));
            read_line("Гос. строй: ", b5, sizeof(b5));
            read_line("Глава государства: ", b6, sizeof(b6));
            db_list_areas(db);
            aid = (int)read_long("area_id (0 если нет): ", 0);
            read_line("Путь к файлу фото (пусто = без фото): ", b7,
                      sizeof(b7));
            {
                unsigned char *blob = NULL;
                int blen = 0;
                if (b7[0]) {
                    if (read_file_blob(b7, &blob, &blen) != 0) {
                        printf("Фото не загружено, вставка без BLOB.\n");
                    }
                }
                if (db_insert_state(db, b1, b2, b3, pop, terr, b4, b5, b6, aid,
                                    blob, blen) != SQLITE_OK)
                    fprintf(stderr, "Ошибка INSERT.\n");
                else
                    printf("Запись добавлена.\n");
                free(blob);
            }
            break;
        case 8: {
            int id = (int)read_long("id для удаления: ", -1);
            if (id < 0)
                printf("Некорректный id.\n");
            else
                db_delete_by_id(db, id);
            break;
        }
        case 9: {
            int id = (int)read_long("id государства: ", -1);
            read_line("Путь выходного файла: ", b1, sizeof(b1));
            if (id >= 0 && b1[0])
                db_export_photo(db, id, b1);
            else
                printf("Нужны id и путь.\n");
            break;
        }
        case 10:
            db_demo_autocommit_inserts(db);
            break;
        case 11:
            db_demo_transaction_inserts(db);
            break;
        default:
            printf("Неизвестный пункт.\n");
        }
    }
}
