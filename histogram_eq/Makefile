.POSIX:
CC     = cc
CFLAGS = -ansi -pedantic -O3 -g3 -Wall -Wextra

histogram_eq: histogram_eq.c bmp.h
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ histogram_eq.c

clean:
	rm -f histogram_eq output.bmp
