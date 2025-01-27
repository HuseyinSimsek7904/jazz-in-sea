/*
This file is part of JazzInSea.

JazzInSea is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

JazzInSea is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with JazzInSea. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _STATE_STATE_GENERATION_H
#define _STATE_STATE_GENERATION_H

#include "state/board_state_t.h"
#include "state/history.h"
#include "board/hash_t.h"

#include <stdbool.h>

void generate_islands(board_state_t* state);
void generate_state_cache(board_state_t* state, history_t* history);

#endif
