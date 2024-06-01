CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Wunused -std=c17
LDFLAGS = -lsqlite3

TARGET = hagelslag

SRCS = main.c connection.c logger.c pool.c queue.c database.c

all: $(TARGET)

$(TARGET): clean $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

debug: CFLAGS += -ggdb3
debug: $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all
