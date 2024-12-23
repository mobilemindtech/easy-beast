CC=gcc
INCLUDES = -I../public
CFLAGS= -Wno-incompatible-pointer-types $(INCLUDES)
LDFLAGS = -L../build -lEasyBeast -pthread
SOURCES = main.c

all: server

server: 
	gcc -lstdc++ -o server main.c $(LDFLAGS) $(CFLAGS)


clean:
	rm -f server server.o
run: server
	./server
