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

#ifndef _MOVE_MOVE_T_H
#define _MOVE_MOVE_T_H

#include "board/board_t.h"
#include "board/piece_t.h"
#include "board/pos_t.h"
#include <stdlib.h>

typedef struct {
  // The position from where the piece moved.
  pos_t from;

  // The position where the piece moved to.
  pos_t to;

  // The position of the piece that the moving piece captured.
  // POSITION_INV if no capture.
  pos_t capture;

  // The type of the piece that the moving piece captured.
  // Does not matter if no capture.
  piece_t capture_piece;
} move_t;

#define MOVE_INV                                                               \
  (move_t) { .from = POSITION_INV }

// Check if two moves are the same.
static inline bool compare_move(move_t move1, move_t move2) {
  if (move1.from != move2.from)
    return false;
  if (move1.to != move2.to)
    return false;
  if (is_valid_pos(move1.capture) ^ is_valid_pos(move2.capture))
    return false;

  return true;
}

// Check if a move is valid.
// This can be checked by checking if the from position is valid.
static inline bool is_valid_move(move_t move) {
  return is_valid_pos(move.from);
}

// Returns if the move is a capture move.
static inline bool is_capture(move_t move) {
  return is_valid_pos(move.capture);
}

// Get a randomly selected move from a MOVE_INV terminated array of moves.
static inline move_t random_move(move_t *moves) {
  size_t length = 0;
  while (is_valid_move(moves[length]))
    length++;
  return moves[rand() % length];
}

#endif
