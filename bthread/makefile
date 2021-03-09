
.PHONY: all clean

OBJS := client bench libbthread.so
CFLAGS := -O1 -ggdb3 -Wall -Wextra -std=gnu99

all: $(OBJS)

client: client.c bthread.c
	gcc $(CFLAGS)  $^ -o $@

bench: benchmark.c bthread.c
	gcc $(CFLAGS)   $^ -o $@

libbthread.so: bthread.c
	gcc $(CFLAGS) -shared -o $@ -fPIC $<

clean:
	rm -f $(OBJS)

