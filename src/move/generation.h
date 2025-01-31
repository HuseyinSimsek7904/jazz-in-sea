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

#ifndef _MOVE_GENERATION_H
#define _MOVE_GENERATION_H

#include "move/move_t.h"
#include "state/board_state_t.h"

#include <stddef.h>

void generate_moves(board_state_t *, move_t[256]);

#endif
