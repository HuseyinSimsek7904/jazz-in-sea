SRC-DIR		?= ./src
INC-DIR		?= ./include
BUILD-DIR	?= ./build
TESTS-DIR	?= ./tests

DEBUG-MACROS	?=	\
-UTEST_EVAL_STATE	\
-UTEST_BOARD_INIT	\

CMACROS		?=	\
-DMEASURE_EVAL_COUNT	\
-DMEASURE_EVAL_TIME	\
-DMM_OPT_MEMOIZATION	\
-UMM_OPT_UPDATE_MEMO	\
-UMM_OPT_AB_PRUNING	\

INSTALL-PATH	:= $(BUILD-DIR)/main

CC		:= gcc
CFLAGS		:= -Wall -Werror $(CMACROS)
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

.PHONY: gdb debug build \
	compile setup-dir install \
	clean \
	gen-bear tests

gdb: CFLAGS += -g $(DEBUG_MACROS)
gdb: compile install

debug: CFLAGS += -O3 $(DEBUG_MACROS)
debug: compile install

build: CFLAGS += -O3 -DNDEBUG
build: compile install

compile: $(OBJ) $(SRC-DIR)/main

setup-dir:
	mkdir -p $(BUILD-DIR)

install: setup-dir
	cp $(SRC-DIR)/main $(BUILD-DIR)/main

clean:
	rm -f $(SRC-DIR)/main $(OBJ)

gen-bear: clean
	bear -- make

tests:
	@for path in $$(ls $(TESTS-DIR)); do	\
		$(TESTS-DIR)/$$path;		\
	done					\
