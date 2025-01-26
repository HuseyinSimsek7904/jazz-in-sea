#ifndef _AI_SEARCH_H
#define _AI_SEARCH_H

#include "ai/cache.h"
#include "ai/eval_t.h"
#include "ai/transposition_table.h"
#include "ai/measure_count.h"
#include "move/move_t.h"
#include "state/board_state_t.h"
#include "state/history.h"
#include "io/pp.h"

eval_t _evaluate(board_state_t *,
                 history_t *,
                 ai_cache_t *,
                 size_t,
                 eval_t,
                 eval_t,
                 move_t*);

#endif
