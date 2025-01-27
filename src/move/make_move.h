/*
This file is part of JazzInSea.

JazzInSea is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

JazzInSea is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with JazzInSea. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _MOVE_MAKE_MOVE_H
#define _MOVE_MAKE_MOVE_H

#include "state/board_state_t.h"
#include "state/history.h"

bool place_piece(board_state_t*, history_t*, pos_t, piece_t);
bool remove_piece(board_state_t*, history_t*, pos_t);

bool do_move(board_state_t*, history_t*, move_t);
bool undo_last_move(board_state_t*, history_t*);

#endif
