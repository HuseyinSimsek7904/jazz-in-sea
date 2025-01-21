SRC-DIR		?= ./src
BUILD-DIR	?= ./build
TESTS-DIR	?= ./scripts/tests

# TEST_EVAL_STATE	Test if the state object is the same before and
#			after call to '_evaluate'.

DEBUG-MACROS	?=	\
-DTEST_EVAL_STATE	\

# MEASURE_EVAL_COUNT	Count the number of calls to the _evaluate function.
# MEASURE_EVAL_TIME	Measure how long the _evaluate function takes.
# MM_OPT_TRANSPOSITION  Use transpositions tables.
# MM_OPT_EXC_DEEPENING  Deepen searching the capture nodes.

CMACROS		?=	\
-DMEASURE_EVAL_COUNT	\
-DMEASURE_EVAL_TIME	\
-DMM_OPT_TRANSPOSITION	\
-UMM_OPT_EXC_DEEPENING	\
-DMM_OPT_ORDERING	\

LDLIBS		:=		\
src/commands/commands.o		\
src/commands/globals.o		\
src/state/history.o		\
src/state/state_generation.o	\
src/state/status.o		\
src/state/hash_operations.o	\
src/io/pp.o			\
src/io/fen.o			\
src/ai/cache.o			\
src/ai/move_ordering.o		\
src/ai/position_evaluation.o	\
src/ai/transposition_table.o	\
src/ai/measure_count.o		\
src/ai/evaluation.o		\
src/ai/search.o			\
src/ai/iterative_deepening.o	\
src/move/make_move.o		\
src/move/generation.o		\

OBJ		:=	\
$(SRC-DIR)/main.o	\
$(LDLIBS)		\

CC		:= @gcc
CFLAGS		:= -Wall -Werror $(CMACROS)
CPPFLAGS	:= -I src/

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
		$(TESTS-DIR)/$$path || :;	\
	done					\
