main: ./src/main.c
	gcc ./src/cursor.c ./src/text-buffer.c ./src/main.c ./src/terminal.c -o ./bin/main -Wall -Wextra -pedantic -std=c2x
