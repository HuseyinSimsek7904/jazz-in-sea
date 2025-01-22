#include "ai/measure_count.h"

#ifdef MEASURE_EVAL_COUNT
size_t evaluate_count = 0;
size_t game_end_count = 0;
size_t leaf_count = 0;
size_t ab_branch_cut_count = 0;

size_t tt_remember_count = 0;
size_t tt_saved_count = 0;
size_t tt_overwritten_count = 0;
size_t tt_rewritten_count = 0;
#endif
