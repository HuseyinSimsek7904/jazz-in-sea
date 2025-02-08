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

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "board/bb_tables.h"
#include "board/board_t.h"
#include "board/piece_t.h"
#include "board/pos_t.h"
#include "io/pp.h"
#include "move/move_t.h"
#include "state/board_state_t.h"
#include "state/hash_operations.h"
#include "state/status.h"

size_t _generate_islands_color(board_state_t *state, uint64_t pieces_bb) {
  size_t total = 0;
  uint64_t old_bb = center_squares_bb & pieces_bb;

  while (old_bb) {
    state->islands_bb |= old_bb;

    uint64_t new_bb = 0;

    while (old_bb) {
      total++;
      pos_t position = __builtin_ctzl(old_bb);
      old_bb &= ~(1ull << position);
      new_bb |= n_table[1][position] & pieces_bb & ~(state->islands_bb);
    }

    old_bb = new_bb;
  }

  return total;
}

// Create the island table.
// This table can later be used to check if a move caused a piece to change an
// island.
void generate_islands(board_state_t *state) {
  // Generate island bitboards for white and black.
  state->islands_bb = 0;
  state->white_island_count =
      _generate_islands_color(state, state->pieces_bb[0] | state->pieces_bb[1]);
  state->black_island_count =
      _generate_islands_color(state, state->pieces_bb[2] | state->pieces_bb[3]);

  // Quick check for if the number of island pieces are greater than the total
  // number of pieces.
  assert(state->white_island_count <= state->white_count);
  assert(state->black_island_count <= state->black_count);
}

// Generate the square hashes table.
static inline void _generate_hash_tables(board_state_t *state) {
  state->turn_hash = (long)rand() << 32 | rand();

  for (pos_t position = 0; position < 64; position++) {
    for (int piece = 0; piece < 4; piece++) {
      state->hash_tables[piece][position] = (long)rand() << 32 | rand();
    }
  }
}

// Generate a state cache from only the information given on the board.
void generate_state_cache(board_state_t *state, history_t *history) {
  // Count the pieces on the board.
  state->white_count = 0;
  state->black_count = 0;

  for (pos_t position = 0; position < 64; position++) {
    char piece_color = get_piece_color(state->board[position]);

    if (piece_color == MOD_WHITE)
      state->white_count++;
    else if (piece_color == MOD_BLACK)
      state->black_count++;
  }

  // Generate the hash tables.
  _generate_hash_tables(state);

  // Generate the hash value for the board.
  generate_full_hash(state);

  // Update the islands and the game status.
  generate_islands(state);
  generate_board_status(state, history);
}
