CFLAGS=-g -Wall
INCLUDES = -I../wrapper -I../xmparser
DEFINE = -DLINUX_PLATFORM
SOURCES = ../wrapper/fs_wrapper_linux.c ../xmparser/xmparser.c

linux:
	gcc $(CFLAGS) $(INCLUDES) $(DEFINE) main.c $(SOURCES) -o test