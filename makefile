SRC-DIR		:= src
INC-DIR		:= include
BUILD-DIR	:= build

CC		:= gcc
CFLAGS		:= -O2 -Wall -Werror
CPPFLAGS	:= -I $(INC-DIR)

LDLIBS		:=	\
src/board.o		\
src/position.o		\
src/move.o		\

OBJ		:=	\
$(SRC-DIR)/main.o	\
$(LDLIBS)		\

.PHONY: build-all link install clean

all: build-all link install

build-all: $(OBJ)

link: $(SRC-DIR)/main

install: $(BUILD-DIR)
	cp $(SRC-DIR)/main $(BUILD-DIR)/main

clean:
	rm -f $(SRC-DIR)/main $(OBJ)

$(BUILD-DIR):
	mkdir $(BUILD-DIR)
