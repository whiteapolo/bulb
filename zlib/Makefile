CC = gcc
BASE_CFLAGS = \
    -I./include \
    -Wall \
    -Wextra \
    -Werror \
    -Wconversion \
    -Wsign-conversion \
    -Wformat=2 \
    -Wcast-qual \
    -Wswitch-enum \
    -Wmissing-prototypes

PROD_CFLAGS = $(BASE_CFLAGS) $(WARN_CFLAGS) -O3
DEV_CFLAGS = $(BASE_CFLAGS) $(WARN_CFLAGS) -O0 -g

all: release

release:
	@mkdir -p obj
	$(CC) $(PROD_CFLAGS) -c ./src/all.c -o obj/libzatar.o
	ar rcs libzatar.a ./obj/libzatar.o

dev:
	@mkdir -p obj
	$(CC) $(DEV_CFLAGS) -c ./src/all.c -o obj/libzatar.o
	ar rcs libzatar.a ./obj/libzatar.o

clean:
	rm -rf obj libzatar.a

.PHONY: all clean release dev
