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

#include "ai/position_evaluation.h"
#include "ai/cache.h"
#include "ai/measure_count.h"
#include "board/board_t.h"
#include "board/piece_t.h"
#include "board/pos_t.h"
#include "io/pp.h"
#include "move/move_t.h"
#include "state/board_state_t.h"
#include <assert.h>
#include <stdbool.h>

// Generate a short "guessed" evaluation score for a move for move ordering.
// Ignore whether or not pieces are in islands.
eval_t get_short_move_evaluation(board_state_t *state, ai_cache_t *cache,
                                 move_t move) {
  piece_t piece = state->board[move.from];
  eval_t evaluation;

  switch (get_piece_type(piece)) {
  case MOD_PAWN:
    evaluation =
        cache->pawn_adv_table[move.to] - cache->pawn_adv_table[move.from];
    break;
  case MOD_KNIGHT:
    evaluation =
        cache->knight_adv_table[move.to] - cache->knight_adv_table[move.from];
    break;
  default:
    assert(false);
    return 0;
  }

  if (is_capture(move)) {
    switch (get_piece_type(move.capture_piece)) {
    case MOD_PAWN:
      evaluation = cache->pawn_adv_table[move.capture];
      break;
    case MOD_KNIGHT:
      evaluation = cache->knight_adv_table[move.capture];
      break;
    default:
      assert(false);
      return 0;
    }
  }

  switch (get_piece_color(piece)) {
  case MOD_WHITE:
    break;
  case MOD_BLACK:
    evaluation = -evaluation;
    break;
  default:
    assert(false);
    return 0;
  }

  return evaluation;
}

// Generate a full evaluation score for the current board.
int get_board_evaluation(board_state_t *state, ai_cache_t *cache) {
#ifdef MEASURE_EVAL_COUNT
  position_evaluation_count++;
#endif

  // If players have centered pieces, add centered advantage score.
  int eval = 0;
  if (state->white_island_count)
    eval += cache->centered_adv;
  if (state->black_island_count)
    eval -= cache->centered_adv;

  // Iterate through all squares and sum up the advantage of each piece.
  for (pos_t position = 0; position < 64; position++) {
    piece_t piece = state->board[position];
    char piece_color = get_piece_color(piece);
    int piece_eval;
    bool centered;

    switch (piece_color) {
    case MOD_WHITE:
      centered = (bool) state->white_island_count;
      break;
    case MOD_BLACK:
      centered = (bool) state->black_island_count;
      break;
    case MOD_EMPTY:
      continue;
    default:
      assert(false);
      continue;
    }

    switch (get_piece_type(piece)) {
    case MOD_PAWN:
      if (centered) {
        piece_eval = ((state->islands_bb & (1ull << position))
                          ? cache->pawn_island_adv_table
                          : cache->pawn_centered_adv_table)[position];
      } else {
        assert(!(state->islands_bb & (1ull << position)));
        piece_eval = cache->pawn_adv_table[position];
      }
      break;

    case MOD_KNIGHT:
      if (centered) {
        piece_eval = ((state->islands_bb & (1ull << position))
                          ? cache->knight_island_adv_table
                          : cache->knight_centered_adv_table)[position];
      } else {
        assert(!(state->islands_bb & (1ull << position)));
        piece_eval = cache->knight_adv_table[position];
      }
      break;

    default:
      assert(false);
      return 0;
    }

    if (piece_color == MOD_BLACK)
      piece_eval = -piece_eval;
    eval += piece_eval;
  }

  return eval;
}

// Get the new evaluation after a move.
int new_evaluation(board_state_t *state, ai_cache_t *cache, move_t move,
                   int old_evaluation, bool update_islands_table) {
  if (update_islands_table)
    return get_board_evaluation(state, cache);

  // Must use move.to as this function must be called after a call to do_move.
  piece_t piece = state->board[move.to];

  // No need to think about the islands cases, as we know that the islands table
  // was not updated. This means neither the piece itself, or the piece it
  // captured was not and is not inside an island.
  int delta = 0;
  switch (get_piece_type(piece)) {
  case MOD_PAWN:
    delta += cache->pawn_adv_table[move.to] - cache->pawn_adv_table[move.from];
    break;
  case MOD_KNIGHT:
    delta +=
        cache->knight_adv_table[move.to] - cache->knight_adv_table[move.from];
    break;
  default:
    assert(false);
    return 0;
  }

  if (is_capture(move)) {
    switch (get_piece_type(move.capture_piece)) {
    case MOD_PAWN:
      delta += cache->pawn_adv_table[move.capture];
      break;
    case MOD_KNIGHT:
      delta += cache->knight_adv_table[move.capture];
      break;
    default:
      assert(false);
      return 0;
    }
  }

  if (get_piece_color(piece) == MOD_BLACK)
    delta = -delta;

  return old_evaluation + delta;
}
