SRC-DIR		:= ./src
INC-DIR		:= ./include
BUILD-DIR	:= ./build
SCRIPTS-DIR	:= ./scripts

# EVALCOUNT enables counting calls to the _evaluate function.
# AB_PRUNING enables alpha-beta pruning.
# MEMOIZATION enables memoization for the AI.

CMACROS		:=	\
-UMM_OPT_MEMOIZATION	\
-UMM_OPT_UPDATE_MEMO	\
-UMM_OPT_AB_PRUNING	\
-DMEASURE_EVAL_COUNT	\
-DMEASURE_EVAL_TIME	\
-UTEST_EVAL_HASH	\
-UTEST_BOARD_INIT	\
-UNDEBUG		\

CC		:= gcc
CFLAGS		:= -O2 -Wall -Werror $(CMACROS)
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

$(BUILD-DIR):
	mkdir $(BUILD-DIR)

generate-compile-commands:
	make clean
	bear -- make

test:
	./tests/branch_test.sh   || :
	./tests/valgrind_test.sh || :
