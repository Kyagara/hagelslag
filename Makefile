CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Wunused -std=c17
TARGET = hagelslag

SRCS = main.c connection.c logger.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
