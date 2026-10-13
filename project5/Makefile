# Лаб. 3, задание 5 — сборка консольного приложения SQLite (вариант 3)
CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11 -O2 -Iinclude
LDFLAGS := -lsqlite3

SRC     := src/main.c src/db.c src/menu.c
OBJ     := $(SRC:.c=.o)
TARGET  := state_app

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET) states_lab3.db

run: $(TARGET)
	./$(TARGET)
