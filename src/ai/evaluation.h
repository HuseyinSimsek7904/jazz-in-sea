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

#ifndef _AI_EVALUATION_H
#define _AI_EVALUATION_H

#include "ai/cache.h"
#include "ai/eval_t.h"
#include "ai/transposition_table.h"
#include "board/board_t.h"
#include "board/pos_t.h"
#include "move/move_t.h"
#include "state/board_state_t.h"
#include "state/history.h"

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

eval_t evaluate(board_state_t *, history_t *, size_t, struct timespec,
                move_t *);

#endif
