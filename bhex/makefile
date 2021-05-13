
CC := gcc


CFLAGS += -std=gnu11 -ggdb3 -Wall -Wextra

CFLAGS += -Wshadow -Wdouble-promotion -Wformat=2
CFLAGS += -Wformat-truncation -Wundef -fno-common
CFLAGS += -Wconversion

CFLAGS += -fsanitize=address -fsanitize=undefined
# https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
# https://developers.redhat.com/blog/2018/03/21/compiler-and-linker-flags-gcc/
# https://gcc.gnu.org/onlinedocs/gcc/Code-Gen-Options.html
# https://gcc.gnu.org/onlinedocs/gcc/index.html#Top
.PHONY: clean

bhex: bhex.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f -- bhex
