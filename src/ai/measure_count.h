#ifndef _AI_MEASURE_COUNT_H
#define _AI_MEASURE_COUNT_H

#include <stddef.h>

#ifdef MEASURE_EVAL_COUNT

extern size_t evaluate_count;
extern size_t game_end_count;
extern size_t leaf_count;
extern size_t ab_branch_cut_count;

#ifdef MM_OPT_TRANSPOSITION
extern size_t tt_remember_count;
extern size_t tt_saved_count;
extern size_t tt_overwritten_count;
extern size_t tt_rewritten_count;
#endif

#endif

#endif
