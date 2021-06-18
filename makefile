OBJ_DIR=build
BIN_DIR=bin
SRC_DIR=src

CC=cc
CFLAGS=-O3 -std=c11 -pedantic -c -Wall

ifeq ($(OS), Windows_NT)
    LDFLAGS=-lglfw3 -lopengl32
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Linux)
		LDFLAGS=-lglfw3 -lGL -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm -pthread -ldl
	endif
	ifeq ($(UNAME_S), Darwin)
		LDFLAGS=-lglfw3 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
	endif
endif

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
