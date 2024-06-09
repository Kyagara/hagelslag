SCANNER ?= HTTP
THREADS ?= 4
QUEUE_LIMIT ?= 256
TASKS_PER_THREAD ?= 16

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Wunused -std=c17 -Ilib/include $(shell pkg-config --cflags libmongoc-1.0)
CFLAGS += -DSCANNER=$(SCANNER) -DTHREADS=$(THREADS) -DQUEUE_LIMIT=$(QUEUE_LIMIT) -DTASKS_PER_THREAD=$(TASKS_PER_THREAD)
LDFLAGS = $(shell pkg-config --libs libmongoc-1.0)

LIB_SRC = $(shell find lib/src -name '*.c')
LIB_OBJ = $(LIB_SRC:.c=.o)
LIB = lib/lib.a

SRC = main.c
OBJ = $(SRC:.c=.o)
HAGELSLAG = hagelslag

all: $(LIB) $(HAGELSLAG)

debug: CFLAGS += -ggdb3
debug: LOG_LEVEL = -1
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
