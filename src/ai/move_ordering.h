/*
This file is part of JazzInSea.

JazzInSea is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

JazzInSea is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with JazzInSea. If not, see <https://www.gnu.org/licenses/>.
*/

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
