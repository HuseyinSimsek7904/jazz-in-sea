/*
This file is part of JazzInSea.

JazzInSea is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

JazzInSea is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
JazzInSea. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _AI_ITERATIVE_DEEPENING_H
#define _AI_ITERATIVE_DEEPENING_H

#include "ai/cache.h"
#include "ai/eval_t.h"
#include "state/board_state_t.h"
#include "state/history.h"

#include <stddef.h>

typedef struct {
  board_state_t *state;
  history_t *history;
  ai_cache_t *cache;
  size_t max_depth;
  move_t *best_moves;
  eval_t *evaluation;
} _id_routine_args_t;

void *_id_routine(void *r_args);

#endif
