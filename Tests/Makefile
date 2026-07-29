CC = gcc

COMPILE_FLAGS = -Wall -Wextra

C_FILES = $(wildcard *.c)
OBJ_FILES = $(patsubst %.c, %.o, $(C_FILES))

include TripleT_UI/TripleT_UI_X11.mk

BIN = main

.DEFAULT_GOAL:= all
all: $(BIN)

$(BIN): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) $(TRIPLET_UI_X11_LIBS) -o $(BIN)  

%.o: %.c
	$(CC) $(COMPILE_FLAGS) -c -o $@ $^

clean:
	rm -rf $(BIN) $(OBJ_FILES)
