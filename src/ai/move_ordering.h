#ifndef _AI_MOVE_ORDERING_H
#define _AI_MOVE_ORDERING_H

#include "ai/cache.h"
#include "ai/eval_t.h"
#include "move/move_t.h"
#include "state/board_state_t.h"

void order_moves(board_state_t*, ai_cache_t*, move_t*, size_t, bool, move_t*);

#endif
