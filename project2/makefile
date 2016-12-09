CC=gcc
CFLAGS=-Wall -I.

download: download.c
	$(CC) download.c tcp.c parse_url.c -o download $(CFLAGS)

clean:
	rm -f download *.o
