#ifndef _AI_MOVE_ORDERING_H
#define _AI_MOVE_ORDERING_H

#include "ai/cache.h"
#include "ai/eval_t.h"
#include "move/move_t.h"
#include "state/board_state_t.h"

typedef struct {
  move_t move;
  eval_t eval;
} move_eval_pair_t;

void order_moves(board_state_t*, ai_cache_t*, move_t*, bool, move_t*);

#endif
