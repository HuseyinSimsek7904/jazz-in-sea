SRC-DIR		:= ./src
INC-DIR		:= ./include
BUILD-DIR	:= ./build
SCRIPTS-DIR	:= ./scripts

CC		:= gcc
CFLAGS		:= -O2 -Wall -Werror -DEVALCOUNT
CPPFLAGS	:= -I $(INC-DIR)

LDLIBS		:=	\
src/board.o		\
src/rules.o		\
src/fen.o		\
src/ai.o		\
src/io.o		\

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

test:
	$(SCRIPTS-DIR)/tests.sh

$(BUILD-DIR):
	mkdir $(BUILD-DIR)

generate-compile-commands:
	make clean
	bear -- make
