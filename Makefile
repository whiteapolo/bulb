# Variables
CC = gcc
CFLAGS = -Wall -Wextra -O3
TARGET = bulb
PREFIX = ~/.local/bin

# Default rule
all: $(TARGET)

# Compile the program
$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

# Clean rule to remove the executable and object files
clean:
	rm -f $(TARGET)

# Install rule to move the executable to ~/.local/bin/
install: $(TARGET)
	mkdir -p $(PREFIX)
	cp $(TARGET) $(PREFIX)/$(TARGET)

# Phony targets to avoid conflicts with file names
.PHONY: all clean install
