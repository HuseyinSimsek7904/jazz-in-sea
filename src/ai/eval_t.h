#ifndef _AI_EVAL_T_H
#define _AI_EVAL_T_H

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>

typedef int eval_t;

#define EVAL_INVALID     (eval_t) INT_MIN
#define EVAL_WHITE_MATES (eval_t) INT_MAX
#define EVAL_BLACK_MATES (eval_t) INT_MIN + 1

static inline bool is_mate(eval_t eval) { return abs(eval) > 1000000; }
static inline int mate_depth(eval_t eval) { return EVAL_WHITE_MATES - abs(eval); }

#endif
