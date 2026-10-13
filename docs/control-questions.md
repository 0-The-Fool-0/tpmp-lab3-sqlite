# Контрольные вопросы по SQLite

## 1. Способы создания базы данных SQLite

1. Через консоль `sqlite3`:
   - `sqlite3 mydb.sqlite`
   - Если файл не существует, он будет создан автоматически.
2. SQL-командой подключения в самой консоли:
   - `.open mydb.sqlite`
3. Из кода (C/C++/Python и т.д.) через API:
   - `sqlite3_open("mydb.sqlite", &db);`
4. Созданием пустого файла вручную:
   - например, `type nul > mydb.sqlite` (Windows), затем открыть через `sqlite3`.

## 2. Команда для просмотра списка баз и подключённых файлов

Используется команда:

```sql
.databases
```

Она показывает список подключённых баз данных (например, `main`, `temp`) и пути к файлам.

## 3. Команды для экспорта данных таблицы в `.csv`

Пример для таблицы `students`:

```sql
.mode csv
.headers on
.once students.csv
SELECT * FROM students;
```

Альтернативно:

```sql
.mode csv
.headers on
.output students.csv
SELECT * FROM students;
.output stdout
```

## 4. Экспорт таблицы и всей БД в `.sql` и в `.sql.tgz`

### Экспорт отдельной таблицы в `.sql`

```sql
.output students.sql
.dump students
.output stdout
```

### Экспорт всей базы в `.sql`

```sql
.output full_backup.sql
.dump
.output stdout
```

### Сжатие в `.sql.tgz` (внешней утилитой)

В Linux/macOS:

```bash
sqlite3 mydb.sqlite ".dump" > full_backup.sql
tar -czf full_backup.sql.tgz full_backup.sql
```

В Windows PowerShell:

```powershell
sqlite3 mydb.sqlite ".dump" > full_backup.sql
tar -czf full_backup.sql.tgz full_backup.sql
```

## 5. Как вывести данные по строкам и по столбцам

- По строкам (list):

```sql
.mode list
SELECT * FROM students;
```

- По столбцам (column):

```sql
.mode column
SELECT * FROM students;
```

## 6. Для чего используется `.headers`

Команда управляет выводом названий столбцов в результате запроса:

- `.headers on` — показывать имена столбцов;
- `.headers off` — скрывать имена столбцов.

Это особенно полезно при выводе в `csv` и `column`.

## 7. Команда для вывода настроек окружения SQLite

Используется:

```sql
.show
```

Команда выводит текущие настройки оболочки (`mode`, `headers`, `separator`, `nullvalue` и т.д.).

## 8. Команда списка таблиц базы данных

```sql
.tables
```

## 9. Пример запроса выборки из 2-х таблиц

Пример с `students` и `groups`:

```sql
SELECT s.id, s.name, g.group_name
FROM students s
JOIN groups g ON s.group_id = g.id;
```

## 10. Пример запроса обновления строк по условию

```sql
UPDATE students
SET scholarship = 5000
WHERE avg_score >= 4.5;
```

Здесь обновляются только строки, где значение поля `avg_score` удовлетворяет условию.

## 11. Пример функции открытия соединения (C)

```c
#include <sqlite3.h>
#include <stdio.h>

sqlite3* open_database(const char* filename) {
    sqlite3* db = NULL;
    int rc = sqlite3_open(filename, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }
    return db;
}
```

Функция открывает БД и возвращает указатель `sqlite3*`, который используется другими функциями для работы с этой БД.

## 12. Пример синтаксиса `sqlite3_exec` и результат выполнения

Синтаксис:

```c
int sqlite3_exec(
    sqlite3* db,
    const char* sql,
    int (*callback)(void*, int, char**, char**),
    void* data,
    char** errmsg
);
```

Пример:

```c
char* errMsg = NULL;
int rc = sqlite3_exec(db,
    "UPDATE students SET scholarship = 7000 WHERE id = 1;",
    NULL, NULL, &errMsg);
```

Результат:
- `rc == SQLITE_OK` — SQL успешно выполнен;
- иначе в `errMsg` будет текст ошибки, его нужно освободить через `sqlite3_free(errMsg)`.

## 13. Функция закрытия соединения после `sqlite3_open`

Используется функция:

```c
int sqlite3_close(sqlite3*);
```

Пример:

```c
int rc = sqlite3_close(db);
if (rc != SQLITE_OK) {
    fprintf(stderr, "Close error\n");
}
```

## 14. Пример кода C для создания таблицы

```c
const char* sql =
    "CREATE TABLE IF NOT EXISTS students ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "name TEXT NOT NULL,"
    "avg_score REAL"
    ");";

char* errMsg = NULL;
int rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", errMsg);
    sqlite3_free(errMsg);
}
```

Объяснение:
- формируется SQL-строка `CREATE TABLE`;
- `IF NOT EXISTS` предотвращает ошибку, если таблица уже есть;
- `sqlite3_exec` выполняет SQL;
- при ошибке читаем `errMsg` и освобождаем память.

## 15. Пример кода C для вставки данных

```c
const char* sql =
    "INSERT INTO students (name, avg_score) "
    "VALUES ('Ivan Petrov', 4.8);";

char* errMsg = NULL;
int rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
if (rc != SQLITE_OK) {
    fprintf(stderr, "Insert error: %s\n", errMsg);
    sqlite3_free(errMsg);
}
```

Объяснение:
- SQL-команда вставляет новую строку в таблицу;
- `sqlite3_exec` выполняет вставку;
- при неуспехе возвращается код ошибки и текст в `errMsg`.

## 16. Пример C-кода с AUTOCOMMIT и TRANSACTION, особенности

```c
char* errMsg = NULL;

/* Явная транзакция */
sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, &errMsg);
sqlite3_exec(db, "INSERT INTO students(name, avg_score) VALUES('Ann', 4.9);", NULL, NULL, &errMsg);
sqlite3_exec(db, "INSERT INTO students(name, avg_score) VALUES('Bob', 4.2);", NULL, NULL, &errMsg);
sqlite3_exec(db, "COMMIT;", NULL, NULL, &errMsg);
```

Особенности:
- **AUTOCOMMIT** (поведение по умолчанию): каждая отдельная SQL-команда фиксируется автоматически после выполнения.
- **TRANSACTION** (`BEGIN ... COMMIT/ROLLBACK`): позволяет объединить несколько операций в одну атомарную группу.
- Преимущества явной транзакции:
  - выше производительность при множественных `INSERT/UPDATE`;
  - целостность данных: либо выполняются все команды, либо при `ROLLBACK` не выполняется ни одна.
