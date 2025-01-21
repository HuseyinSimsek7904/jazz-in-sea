#ifndef _AI_POSITION_EVALUATION_H
#define _AI_POSITION_EVALUATION_H

#include "ai/cache.h"
#include "state/board_state_t.h"
#include "move/move_t.h"

eval_t get_short_move_evaluation(board_state_t* state, ai_cache_t* cache, move_t move);
int get_board_evaluation(board_state_t *state, ai_cache_t *cache);

#endif
