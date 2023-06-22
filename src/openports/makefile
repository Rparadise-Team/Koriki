CC = gcc
CFLAGS = -Wall -Wextra

.PHONY: all clean

all: openport

openport: openport.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f openport