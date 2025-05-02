CC = gcc
INC_FLAGS = -Iinclude
CFLAGS = -Wall -ansi -pedantic $(INC_FLAGS)
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
EXEC = assembler

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

clean:
	rm -f src/*.o $(EXEC) output/* examples/*.am examples/*.nolabels

run:
	./assembler examples/example1.as

.PHONY: all clean run
