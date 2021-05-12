

CFLAGS += -Wall -Wextra -Werror -Wpedantic -std=gnu17 -Wconversion -ggdb3
LDFLAGS += -ldl -rdynamic

bench: bperf.c bench.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

