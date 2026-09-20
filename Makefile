CC = cc
RELEASE_CFLAGS = -Wall -Wextra -O3 -I./zlib/include
DEV_CFLAGS = -Wall -Wextra -O0 -g -I./zlib/include
TARGET = bulb
PREFIX = /usr/local/bin

all: release

release:
	make -C zlib
	$(CC) $(RELEASE_CFLAGS) -o $(TARGET) main.c ./zlib/libzatar.a

dev:
	make dev -C zlib
	$(CC) $(DEV_CFLAGS) -o $(TARGET) main.c ./zlib/libzatar.a

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
