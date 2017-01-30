TARGET = mkupgimg.exe

CC = gcc
LD = $(CC)
CFLAGS=-c -std=gnu99

mkupgimg: mkupgimg.c
	$(CC) -o $@ $^

clean:
	rm -f mkupgimg
