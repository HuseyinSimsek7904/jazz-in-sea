/*
This file is part of JazzInSea.

JazzInSea is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

JazzInSea is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with JazzInSea. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _IO_FEN_H
#define _IO_FEN_H

#include "board/board_t.h"
#include "state/board_state_t.h"
#include "state/history.h"

#include <stdbool.h>

#define DEFAULT_BOARD "np4PN/pp4PP/8/8/8/8/PP4pp/NP4pn w"

bool load_fen_string(const char *, board_state_t *, history_t *);
char *get_fen_string(char *, board_state_t *);

bool load_fen_from_path(const char *, board_state_t *, history_t *);
bool save_fen_to_path(const char *, board_state_t *);

#endif
