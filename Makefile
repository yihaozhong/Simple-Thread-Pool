CC=gcc
CFLAGS=-g  -pedantic -std=gnu17 -Wall -Werror -Wextra
LDFLAGS=-pthread

.PHONY: all
all: simTP

simTP: simTP.o 

simTP.o: simTP.c 
.PHONY: clean
clean:
	rm -f *.o simTP
