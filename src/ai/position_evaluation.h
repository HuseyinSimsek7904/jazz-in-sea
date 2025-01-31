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

#ifndef _AI_POSITION_EVALUATION_H
#define _AI_POSITION_EVALUATION_H

#include "ai/cache.h"
#include "move/move_t.h"
#include "state/board_state_t.h"

eval_t get_short_move_evaluation(board_state_t *state, ai_cache_t *cache,
                                 move_t move);
int get_board_evaluation(board_state_t *state, ai_cache_t *cache);
int new_evaluation(board_state_t *state, ai_cache_t *cache, move_t move,
                   int old_evaluation, bool update_islands_table);

#endif
