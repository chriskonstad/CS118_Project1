CC=g++
CFLAGS=-Wall -Wpedantic

all: server

server: build/main.o
	$(CC) build/main.o -o server

build/main.o: src/main.cpp
	$(CC) $(CFLAGS) src/main.cpp

clean:
	rm build/*.o server
