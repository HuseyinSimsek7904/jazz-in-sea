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

#include "move/generation.h"
#include "ai/measure_count.h"
#include "board/n_table.h"
#include "board/pos_t.h"
#include "board/status_t.h"
#include "io/pp.h"
#include "move/move_t.h"
#include "state/board_state_t.h"

#include <assert.h>
#include <inttypes.h>
#include <stddef.h>

static inline bool sum_inrange(pos_t pos, int delta) {
  int row = to_row(pos);
  int col = to_col(pos);

  int drow = delta / 8;
  int dcol = delta % 8;

  return 0 <= row + drow && row + drow < 8 && 0 <= col + dcol && col + dcol < 8;
}

// Generate all possible moves on the board, and place them on the moves array.
// Moves array is terminated by adding a MOVE_INV.
void generate_moves(board_state_t *state, move_t moves[256]) {
#ifdef MEASURE_EVAL_COUNT
  move_generation_count++;
#endif

  if (state->status != NORMAL) {
    assert(false);
    moves[0] = MOVE_INV;
    return;
  }

  char piece_color = MOD_BLACK;
  char opposite_color = MOD_WHITE;

  if (state->turn) {
    piece_color = MOD_WHITE;
    opposite_color = MOD_BLACK;
  }

  bool capture_available = false;
  size_t length = 0;

  for (pos_t position = 0; position < 64; position++) {
    piece_t piece = state->board[position];

    // Check if the color of the piece is the color of the player.
    if (get_piece_color(piece) != piece_color)
      continue;

    bool is_knight = get_piece_type(piece) == MOD_KNIGHT;

    int deltas[4] = {-1, 1, -8, 8};
    for (int i = 0; i < 4; i++) {
      int delta = deltas[i];

      if (!sum_inrange(position, delta))
        continue;

      int first_pos = position + delta;

      if (is_knight) {
        if (!sum_inrange(first_pos, delta))
          continue;

        first_pos += delta;
      }

      piece_t first_piece = state->board[first_pos];

      if (first_piece == MOD_EMPTY) {
        // The destination position is empty.
        // If there are any available captures, no need to try to find a
        // regular move.
        if (capture_available)
          continue;

        // Set the move object.
        moves[length++] = (move_t){
            .from = position,
            .to = first_pos,
            .capture = POSITION_INV,
        };

      } else if (get_piece_color(first_piece) == opposite_color) {
        // The destination position has a piece of opposite color.
        if (!sum_inrange(first_pos, delta))
          continue;

        int second_pos = first_pos + delta;

        // Check if the destination position is empty.
        if (state->board[second_pos] != EMPTY)
          continue;

        // If we have not found any captures yet, clear all previous moves.
        if (!capture_available) {
          capture_available = true;
          length = 0;
        }

        // Set the move object.
        moves[length++] = (move_t){.from = position,
                                   .to = second_pos,
                                   .capture = first_pos,
                                   .capture_piece = first_piece};
      }
    }
  }
  moves[length] = MOVE_INV;
}
