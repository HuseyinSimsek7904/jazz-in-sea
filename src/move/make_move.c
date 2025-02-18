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

#include "move/make_move.h"
#include "board/bb_tables.h"
#include "board/pos_t.h"
#include "io/pp.h"
#include "state/hash_operations.h"
#include "state/state_generation.h"
#include "state/status.h"
#include <stdint.h>

// Remove a piece on the board.
// Returns the removed piece.
// NOTE: Does not alter the state cache.
static inline piece_t _remove_piece(board_state_t *state, pos_t from,
                                    bool *update_islands_table) {
  // If the piece was moved from an island, table should be updated.
  if (!*update_islands_table && (state->islands_bb & (1ull << from)))
    *update_islands_table = true;

  // Take the piece from the origin position.
  piece_t piece = state->board[from];

  // Clear the origin position.
  state->board[from] = EMPTY;
  state->pieces_bb[piece - 4] &= ~(1ull << from);

  // Return the removed piece.
  return piece;
}

// Place a piece to a position.
// NOTE: Does not alter the state cache.
static inline void _place_piece(board_state_t *state, pos_t to, piece_t piece,
                                bool *update_islands_table) {
  // Set the destination position.
  state->board[to] = piece;
  state->pieces_bb[piece - 4] |= 1ull << to;

  if (!*update_islands_table && is_center(to))
    *update_islands_table = true;

  // Check for all the N1 neighbors of the new position.
  if (!*update_islands_table) {
    uint64_t n1 = n_table[1][to];
    while (n1) {
      pos_t new_pos = __builtin_ctzl(n1);
      n1 &= ~(1ull << new_pos);

      if (get_piece_color(state->board[new_pos]) == get_piece_color(piece) &&
          (state->islands_bb & (1ull << new_pos))) {

        *update_islands_table = true;
        break;
      }
    }
  }
}

// Remove a piece from the board.
// Clears the history.
bool remove_piece(board_state_t *state, history_t *history, pos_t pos) {
  bool update_islands_table = false;
  piece_t piece = _remove_piece(state, pos, &update_islands_table);

  state->hash ^= get_hash_for_piece(state, piece, pos);
  char color = get_piece_color(piece);

  if (color == MOD_WHITE)
    state->white_count--;
  else if (color == MOD_BLACK)
    state->black_count--;
  else
    return false;

  if (update_islands_table)
    generate_islands(state);

  history->size = 0;
  generate_board_status(state, history);

  return true;
}

// Place a piece on the board.
// Clears the history.
bool place_piece(board_state_t *state, history_t *history, pos_t pos,
                 piece_t piece) {
  state->hash ^= get_hash_for_piece(state, piece, pos);
  char color = get_piece_color(piece);

  if (color == MOD_WHITE)
    state->white_count++;
  else if (color == MOD_BLACK)
    state->black_count++;
  else
    return false;

  if (state->board[pos] != EMPTY)
    return false;

  bool update_islands_table = false;
  _place_piece(state, pos, piece, &update_islands_table);

  if (update_islands_table)
    generate_islands(state);

  history->size = 0;
  generate_board_status(state, history);

  return true;
}

// Make a move on the state->board and update the state of the state->board.
// Both the state->board and move objects are assumed to be valid, so no checks
// are performed.
bool do_move(board_state_t *state, history_t *history, move_t move) {
  // Add the move and the old state->board to the history.
  history->history[history->size++] = (history_item_t){
      .move = move,
      .hash = state->hash,
  };

  // There must not be any piece on the position where we are moving the piece.
  assert(state->board[move.to] == EMPTY);

  bool update_islands_table = false;

  // Low level move the piece.
  // Piece color should be state->board->turn.
  piece_t piece = _remove_piece(state, move.from, &update_islands_table);
  assert(get_piece_color(piece) == (state->turn ? MOD_WHITE : MOD_BLACK));
  _place_piece(state, move.to, piece, &update_islands_table);

  // Update the hash value for the move.
  state->hash ^= get_hash_for_move(state, piece, move);

  // If the move is a capture move, remove the piece.
  // There must be a piece where we are going to capture of type capture_piece.
  if (is_capture(move)) {
// Wtf is this!?
#ifndef NDEBUG
    piece_t remove_piece =
#endif

        _remove_piece(state, move.capture, &update_islands_table);
    assert(move.capture_piece == remove_piece);

    char capture_color = get_piece_color(move.capture_piece);

    // Decrement the piece count.
    if (capture_color == MOD_WHITE)
      state->white_count--;
    else if (capture_color == MOD_BLACK)
      state->black_count--;
    else
      assert(false);
  }

  // Update the turn.
  state->turn = !state->turn;

  // Update the necessary caches.
  if (update_islands_table) {
    generate_islands(state);
  }

  generate_board_status(state, history);

  return update_islands_table;
}

// Undo a move on the state->board and update the state of the state->board.
bool undo_last_move(board_state_t *state, history_t *history) {
  // There must be at least one move in the history.
  assert(history->size > 0);

  // Get the last move from the history.
  move_t move = history->history[--history->size].move;

  bool update_islands_table = false;

  // Low level move the piece.
  // Piece color should be !state->board->turn.
  piece_t piece = _remove_piece(state, move.to, &update_islands_table);
  assert(get_piece_color(piece) == (state->turn ? MOD_BLACK : MOD_WHITE));
  _place_piece(state, move.from, piece, &update_islands_table);

  // Update the hash value for the move.
  state->hash ^= get_hash_for_move(state, piece, move);

  // If the move is a capture move, add the piece.
  // There must be no piece where we are going to add the piece.
  if (is_capture(move)) {
    _place_piece(state, move.capture, move.capture_piece,
                 &update_islands_table);

    char capture_color = get_piece_color(move.capture_piece);

    // Increment the piece count.
    if (capture_color == MOD_WHITE) {
      state->white_count++;
    } else if (capture_color == MOD_BLACK) {
      state->black_count++;
    } else
      assert(false);
  }

  // Update the turn.
  state->turn = !state->turn;

  // Update the necessary caches.
  if (update_islands_table) {
    generate_islands(state);
  }

  generate_board_status(state, history);

  return update_islands_table;
}
