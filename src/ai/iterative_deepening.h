#ifndef _AI_ITERATIVE_DEEPENING_H
#define _AI_ITERATIVE_DEEPENING_H

#include "ai/eval_t.h"
#include "state/board_state_t.h"
#include "state/history.h"
#include "ai/cache.h"

#include <stddef.h>

typedef struct {
  board_state_t* state;
  history_t* history;
  ai_cache_t* cache;
  size_t max_depth;
  move_t* best_moves;
  size_t* best_moves_length;
  eval_t* evaluation;
} _id_routine_args_t;

void* _id_routine(void* r_args);

#endif
