CFLAGS := $(CFLAGS) -Wall -O2 -mtune=native -g
INC    := -Iinclude $(INC)
LFLAGS := -levent
CC     := gcc
BINARY := mcvhost-proxy
DEPS   := build/main.o build/debug.o

.PHONY: all clean

all: build $(DEPS) bin/$(BINARY)

build:
	-mkdir -p build bin

build/main.o: src/main.c
	$(CC) $(CFLAGS) $(INC) -c -o build/main.o src/main.c

build/debug.o: src/debug.c include/debug.h
	$(CC) $(CFLAGS) $(INC) -c -o build/debug.o src/debug.c

bin/$(BINARY): $(DEPS)
	$(CC) $(CFLAGS) $(INC) -o bin/$(BINARY) $(DEPS) $(LFLAGS)

clean:
	rm -rfv build bin

clang:
	$(MAKE) CC=clang
