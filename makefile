# Build paths
OBJ_DIR=build/
BIN_DIR=bin/
SRC_DIR=src/
INC_DIR=include/

# Standard Build Flags
CC=gcc
CFLAGS=-O3 -std=c99 -pedantic -c -Wall -I $(INC_DIR)

ifeq ($(OS), Windows_NT)
    LDFLAGS=-lglfw3dll -lopengl32
    BIN=win32/foxxgb
else
	UNAME_S := $(shell uname -s)

	ifeq ($(UNAME_S), Linux)
		LDFLAGS=-lglfw3 -lGL -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm -pthread -ldl
		BIN=linux/foxxgb
	endif

	ifeq ($(UNAME_S), Darwin)
	    LDFLAGS=-lglfw3 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
	    BIN=osx/foxxgb
	endif
endif

# Files
SRC=$(wildcard $(SRC_DIR)*.c)
OBJ=$(addprefix $(OBJ_DIR), $(SRC:$(SRC_DIR)%.c=%.o))

# Build
all: $(SRC) $(addprefix $(BIN_DIR), $(BIN))

$(addprefix $(BIN_DIR), $(BIN)): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJ)
