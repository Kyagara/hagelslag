CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Wunused -std=c17
LDFLAGS =

TARGET = hagelslag

SRCS = main.c connection.c logger.c pool.c queue.c file.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

debug: CFLAGS += -ggdb3
debug: $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
