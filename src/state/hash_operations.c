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

#include "state/hash_operations.h"
#include "board/board_t.h"
#include "board/hash_t.h"
#include "board/piece_t.h"
#include "board/pos_t.h"
#include "state/board_state_t.h"

// Generate the hash value for a board.
void generate_full_hash(board_state_t *state) {
  state->hash = state->turn ? state->turn_hash : 0;
  for (pos_t position = 0; position < 64; position++) {
    piece_t piece = state->board[position];

    if (piece != EMPTY)
      state->hash ^= get_hash_for_piece(state, piece, position);
  }
}
