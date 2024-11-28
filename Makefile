SOURCES := $(shell find ./src -name '*.c')
CC := gcc

main: ./src/main.c
	mkdir -p ./bin
	$(CC) $(SOURCES) -o ./bin/main -Wall -Wextra -pedantic -std=c2x
