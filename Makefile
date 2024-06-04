CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Wunused -std=c17 -Ilib/include
LDFLAGS = -lsqlite3

LIB_SRC = lib/src/connection.c lib/src/database.c lib/src/threadpool.c lib/src/logger.c lib/src/queue.c lib/src/ip.c
LIB_OBJ = $(LIB_SRC:.c=.o)
LIB = lib/lib.a

SRC = main.c
OBJ = $(SRC:.c=.o)
HAGELSLAG = hagelslag

all: $(LIB) $(HAGELSLAG)

debug: CFLAGS += -ggdb3
debug: $(HAGELSLAG)

clean:
	rm -f lib/src/*.o lib/lib.a *.o hagelslag

$(LIB): $(LIB_OBJ)
	ar rcs $@ $^

$(HAGELSLAG): $(OBJ) $(LIB)
	$(CC) $(OBJ) lib/lib.a $(LDFLAGS) -o $@

lib/src/%.o: lib/src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

hagelslag/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all debug clean
