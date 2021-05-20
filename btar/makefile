
CC := gcc


include safe-flags.mk

.PHONY: clean

btar: btar.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f -- btar
