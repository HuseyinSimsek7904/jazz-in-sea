SRC-DIR		?= ./src
INC-DIR		?= ./include
BUILD-DIR	?= ./build
TESTS-DIR	?= ./tests

# TEST_EVAL_STATE	Test if the state object is the same before and
#			after call to '_evaluate'.
# TEST_BOARD_INIT	Test if the board object is initialized before
#			being accessed.

DEBUG-MACROS	?=	\
-UTEST_EVAL_STATE	\
-UTEST_BOARD_INIT	\

# MEASURE_EVAL_COUNT	Count the number of calls to the _evaluate function.
# MEASURE_EVAL_TIME	Measure how long the _evaluate function takes.
# MM_OPT_MEMOIZATION	Use the memoizatino technique to speed the AI up.
# MM_OPT_UPDATE_MEMO	While memorizing, check if an outdated memorized
#			information exist. If so, update it. Kept only for
#			tests and will be removed.
# MM_OPT_AB_PRUNING	Use the alpha beta pruning technique to speed the AI
#			up.
# MM_OPT_EXC_DEEPENING  Deepen searching the capture nodes.

CMACROS		?=	\
-DMEASURE_EVAL_COUNT	\
-DMEASURE_EVAL_TIME	\
-DMM_OPT_MEMOIZATION	\
-UMM_OPT_UPDATE_MEMO	\
-DMM_OPT_AB_PRUNING	\
-UMM_OPT_EXC_DEEPENING	\

LDLIBS		:=	\
src/board.o		\
src/rules.o		\
src/fen.o		\
src/ai.o		\
src/io.o		\

OBJ		:=	\
$(SRC-DIR)/main.o	\
$(LDLIBS)		\

CC		:= @gcc
CFLAGS		:= -Wall -Werror $(CMACROS)
CPPFLAGS	:= -I $(INC-DIR)

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
	@mkdir -p $(BUILD-DIR)

install: setup-dir
	@cp -f $(SRC-DIR)/main $(BUILD-DIR)/main

clean:
	@rm -f $(SRC-DIR)/main $(OBJ)

gen-bear: clean
	@bear -- make

tests:
	@for path in $$(ls $(TESTS-DIR)); do	\
		$(TESTS-DIR)/$$path;		\
	done					\
