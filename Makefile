CC      = gcc
CFLAGS  = -Wall -Wextra -g
TARGET  = shell

all: $(TARGET)

$(TARGET): shell.c
	$(CC) $(CFLAGS) -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
