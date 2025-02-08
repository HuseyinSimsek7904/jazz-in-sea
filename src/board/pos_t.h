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

#ifndef _BOARD_POSITION_T_H
#define _BOARD_POSITION_T_H

#include "board/bb_tables.h"
#include <stdbool.h>
#include <sys/types.h>

// Integers in [0, 64) represent the valid positions.
// Anything outside this boundary counts as invalid position.
#define POSITION_INV (pos_t)64

typedef u_int8_t pos_t;

// Convert from (row, col) to position.
static inline pos_t to_position(int row, int col) {
  // Encode position as:
  //  7  6  5  4  3  2  1  0
  //  0  0 r2 r1 r0 c2 c1 c0
  return (row & 0x07) << 3 | (col & 0x07);
}

// Convert from position to row.
static inline int to_row(pos_t pos) { return (pos >> 3) & 7; }

// Convert from position to column.
static inline int to_col(pos_t pos) { return pos & 7; }

// Check if a position is valid aka it is in the bounds.
static inline bool is_valid_pos(pos_t pos) { return pos < 64; }

static inline bool is_center(pos_t pos) {
  return center_squares_bb & (1ull << pos);
}

#endif
