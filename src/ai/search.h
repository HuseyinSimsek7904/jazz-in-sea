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

#ifndef _AI_SEARCH_H
#define _AI_SEARCH_H

#include "ai/cache.h"
#include "ai/eval_t.h"
#include "ai/measure_count.h"
#include "ai/transposition_table.h"
#include "io/pp.h"
#include "move/move_t.h"
#include "state/board_state_t.h"
#include "state/history.h"

eval_t _evaluate(board_state_t *, history_t *, ai_cache_t *, size_t, int,
                 eval_t, eval_t, move_t *);

#endif
