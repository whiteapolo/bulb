CC = cc
RELEASE_CFLAGS = -Wall -Wextra -O3
DEV_CFLAGS = -Wall -Wextra -O0 -g
TARGET = bulb
PREFIX = /usr/local/bin

all: release

release:
	$(CC) $(RELEASE_CFLAGS) -o $(TARGET) main.c libzatar/libzatar.o

dev:
	$(CC) $(DEV_CFLAGS) -o $(TARGET) main.c libzatar/libzatar.o

clean:
	rm -f $(TARGET)

udev:
	cp ./90-backlight.rules /usr/lib/udev/rules.d/

install: release udev
	mkdir -p $(PREFIX)
	cp $(TARGET) $(PREFIX)/$(TARGET)

uninstall:
	rm -f $(PREFIX)/$(TARGET)

.PHONY: all dev release clean install uninstall udev
