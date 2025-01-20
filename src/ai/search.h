#ifndef _AI_SEARCH_H
#define _AI_SEARCH_H

#include "ai/cache.h"
#include "ai/eval_t.h"
#include "ai/transposition_table.h"
#include "ai/measure_count.h"
#include "state/board_state_t.h"
#include "state/history.h"
#include "io/pp.h"

eval_t _evaluate(board_state_t *state, history_t *history, ai_cache_t *cache,
                 size_t max_depth, move_t *best_moves, size_t *best_moves_length,
                 eval_t alpha, eval_t beta, bool starting_move);

#endif
