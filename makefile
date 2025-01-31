SRCDIR		?= ./src
OBJDIR		?= ./obj
BINDIR		?= ./bin
SCRIPTDIR	?= ./scripts
TESTDIR		?= $(SCRIPTDIR)/tests

EXECUTABLE	?= $(BINDIR)/jazzinsea

SOURCES		:= $(shell find $(SRCDIR) -name '*.c')
OBJECTS		:= $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
OBJDIRS		:= $(sort $(dir $(OBJECTS)))
DEPENDS		:= $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.d, $(SOURCES))

# TEST_EVAL_STATE	Test if the state object is the same before and
#			after call to '_evaluate'.

DEBUGMACROS	?=	\
-UTEST_EVAL_STATE	\

# MEASURE_EVAL_COUNT	Count the number of calls to the _evaluate function.
# MEASURE_EVAL_TIME	Measure how long the _evaluate function takes.

CMACROS		?=	\
-DMEASURE_EVAL_COUNT	\
-DMEASURE_EVAL_TIME	\

CC		:= gcc
CFLAGS		:= -Wall -Werror
CPPFLAGS	:= -I src/ $(CMACROS)

.PHONY: all debug build \
	clean gen-bear tests

all: build

debug: CFLAGS += -g
debug: CPPFLAGS += $(DEBUGMACROS)
debug: $(OBJDIRS) $(EXECUTABLE)

build: CFLAGS += -O3
build: CPPFLAGS += -DNDEBUG
build: $(OBJDIRS) $(EXECUTABLE)

# Header dependencies
-include $(DEPENDS)

# Binary files
$(BINDIR):
	mkdir -p $(BINDIR)

$(EXECUTABLE): $(OBJECTS) | $(BINDIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@

# Object files
$(OBJDIRS):
	mkdir -p $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

# Miscellaneous
clean:
	rm -rf $(OBJDIR) $(BINDIR)

gen-bear: clean
	bear -- make

tests:
	@for path in $$(ls $(TESTDIR)); do	\
		$(TESTDIR)/$$path || :;	\
	done					\
