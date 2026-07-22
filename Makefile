CC=gcc
CFLAGS=-Wall -g

cs240Elections: main.c elections.c elections.h
	$(CC) $(CFLAGS) main.c -o $@

.PHONY: clean

clean:
	rm -f cs240Elections
