OBJ_DIR=build
BIN_DIR=bin
SRC_DIR=src

CC=cc
CFLAGS=-O3 -std=c11 -pedantic -c -Wall -g `llvm-config --cflags`
LDFLAGS=-lglfw3 -lopengl32 `llvm-config --ldflags` `llvm-config --libs`

SRC=$(wildcard $(SRC_DIR)/*.c)
OBJ=$(addprefix $(OBJ_DIR)/, $(notdir $(SRC:.c=.o)))
BIN=foxx-gb

all: $(SRC) $(BIN_DIR)/$(BIN)

$(BIN_DIR)/$(BIN): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJ)
