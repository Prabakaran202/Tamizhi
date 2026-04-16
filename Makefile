CC = gcc
CFLAGS = -Iinclude -Wall
SRC = src/main.c src/lexer.c src/parser.c
OBJ = tamizhi

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OBJ)

clean:
	rm -f $(OBJ)
