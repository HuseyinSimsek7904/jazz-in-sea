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

#ifndef _STATE_BOARD_STATE_T_H
#define _STATE_BOARD_STATE_T_H

#include "board/hash_t.h"
#include "board/piece_t.h"
#include "board/status_t.h"

#include <stdint.h>

typedef struct {
  // The current board.
  piece_t board[64];

  // The current color to move.
  bool turn;

  // The current board status.
  status_t status;

  // Number of pieces of both players.
  unsigned int white_count;
  unsigned int black_count;

  // Number of pieces that are in islands for both players.
  unsigned int white_island_count;
  unsigned int black_island_count;

  // Piece bitboard tables.
  // For non empty pieces, (piece - 4) returns the index in this bitboard.
  uint64_t pieces_bb[4];

  // Islands bitboard table.
  uint64_t islands_bb;

  // Square hashes table used to generate a hash value for boards.
  hash_t hash_tables[4][64];
  hash_t turn_hash;

  // The current hash value.
  hash_t hash;
} board_state_t;

#endif
