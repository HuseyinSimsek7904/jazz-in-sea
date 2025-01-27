/*
This file is part of JazzInSea.

JazzInSea is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

JazzInSea is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with JazzInSea. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _BOARD_PIECE_T_H
#define _BOARD_PIECE_T_H

#include <stdbool.h>
#include <sys/types.h>

#define MOD_EMPTY  0
#define MOD_WHITE  4
#define MOD_BLACK  6
#define MOD_PAWN   4
#define MOD_KNIGHT 5

enum {
  EMPTY        = MOD_EMPTY,
  WHITE_PAWN   = MOD_WHITE | MOD_PAWN,
  WHITE_KNIGHT = MOD_WHITE | MOD_KNIGHT,
  BLACK_PAWN   = MOD_BLACK | MOD_PAWN,
  BLACK_KNIGHT = MOD_BLACK | MOD_KNIGHT,
};

typedef u_int8_t piece_t;

static inline char get_piece_color(piece_t piece) { return piece & 6; }
static inline char get_piece_type(piece_t piece) { return piece & 5; }

#endif
