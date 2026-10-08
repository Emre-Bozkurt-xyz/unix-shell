CC      = gcc
CFLAGS  = -Wall -Wextra -g
TARGET  = osh

all: $(TARGET)

$(TARGET): osh.c
	$(CC) $(CFLAGS) -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
