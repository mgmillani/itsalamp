CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -I$(INC) `pkg-config --cflags gtk+-2.0`
SRC = src
OBJ = obj
BIN = bin
INC = include
LIBS = `pkg-config --libs gtk+-2.0` -lm
OBJECTS = $(OBJ)/config.o $(OBJ)/parse.o $(OBJ)/image.o $(OBJ)/main.o
NAME = itsalamp

all: $(OBJ) $(BIN) release

$(OBJ):
	mkdir $(OBJ)
$(BIN):
	mkdir $(BIN)

release: CFLAGS += -s -O3 -D TRACE_OFF
release: main countdown

debug: CFLAGS += -g -O0
debug: main countdown

countdown: $(OBJ)/countdown.o $(OBJ)/color.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $(BIN)/countdown

main: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) -o $(BIN)/$(NAME)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -f $(OBJ)/*.o
	rm -f $(BIN)/$(NAME)
	rm -f $(BIN)/countdown
