#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "io.h"
#include "rules.h"
#include "board.h"
#include "move.h"
#include "position.h"
#include "piece.h"

// Direction units.
// These macros are defined using the perspective of the white player.
#define UP    -0x10
#define DOWN  +0x10
#define LEFT  -0x01
#define RIGHT +0x01

const int deltas[] = { RIGHT, DOWN, LEFT, UP };

// Generate all possible moves on the board, and place them on the moves array.
// Returns the number of moves.
size_t generate_moves(board_t* board, move_t moves[256]) {
  char piece_color = MOD_BLACK;
  char opposite_color = MOD_WHITE;

  if (board->turn) {
    piece_color = MOD_WHITE;
    opposite_color = MOD_BLACK;
  }

  bool capture_available = false;
  int length = 0;

  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      unsigned int position = to_position(row, col);
      piece_t piece = get_piece(board, position);

      // Check if the color of the piece is the color of the player.
      if (get_piece_color(piece) != piece_color) continue;

      // Calculate the delta length of regular moves.
      int delta_mul = 1;
      if (get_piece_type(piece) == MOD_KNIGHT) {
        delta_mul = 2;
      }

      // Loop through all directions.
      for (size_t i=0; i<4; i++) {
        int delta = deltas[(i + get_quadrant(position)) % 4];

        // Calculate the first position and check if it is valid.
        int first_pos = position + delta * delta_mul;
        if (!is_valid_pos(first_pos)) continue;

        piece_t first_piece = get_piece(board, first_pos);

        if (first_piece == MOD_EMPTY) {
          // The destination position is empty.
          // If there are any available captures, no need to try to find a regular move.
          if (capture_available) continue;

          // Set the move object.
          moves[length++] = (move_t) {
            .from = position,
            .to = first_pos,
            .capture = INV_POSITION,
          };

        } else if (get_piece_color(first_piece) == opposite_color) {
          // The destination position has a piece of opposite color and check if it is valid.
          int second_pos = first_pos + delta;
          if (!is_valid_pos(second_pos)) continue;

          // Check if the destination position is empty.
          if (get_piece(board, second_pos) != EMPTY) continue;

          // If we have not found any captures yet, clear all previous moves.
          if (!capture_available) {
            capture_available = true;
            length = 0;
          }

          // Set the move object.
          moves[length++] = (move_t) {
            .from = position,
            .to = second_pos,
            .capture = first_pos,
            .capture_piece = first_piece
          };
        }
      }
    }
  }

  return length;
}

// As can be seen very clearly, this function is written very poorly.
// This is because this function does not need to be fast right now, we will be
// improving all of the functions as we progress.
// Get the board status.
void _generate_board_status(board_t* board, state_cache_t* state) {
  // Check for insufficient material.
  if (!state->white_count) {
    state->status = state->black_count ? BLACK_WON_BY_INSUF_MAT : DRAW_BY_INSUF_MAT;
    return;
  }

  if (!state->black_count) {
    state->status = WHITE_WON_BY_INSUF_MAT;
    return;
  }

  // Check for islands.
  if (state->white_count == state->white_island_count) {
    state->status = (state->black_count == state->black_island_count) ? DRAW_BY_BOTH_ISLANDS : WHITE_WON_BY_ISLANDS;
    return;
  }

  if (state->black_count == state->black_island_count) {
    state->status = BLACK_WON_BY_ISLANDS;
    return;
  }

  state->status = NORMAL;
}

const char* board_status_text(status_t state) {
  const char* text = "<unknown state>";

  switch (state) {
  case NORMAL:
    text = "continue";
    break;
  case DRAW_BY_REPETITION:
    text = "draw by repetition";
    break;
  case DRAW_BY_BOTH_ISLANDS:
    text = "draw by both players have islands";
    break;
  case DRAW_BY_NO_MOVES:
    text = "draw by no moves available";
    break;
  case DRAW_BY_INSUF_MAT:
    text = "draw by both sides have insufficient material";
    break;
  case WHITE_WON_BY_ISLANDS:
    text = "white won by islands";
    break;
  case WHITE_WON_BY_INSUF_MAT:
    text = "white won as opponent has insufficient material";
    break;
  case BLACK_WON_BY_ISLANDS:
    text = "black won by islands";
    break;
  case BLACK_WON_BY_INSUF_MAT:
    text = "black won as opponent has insufficient material";
    break;
  }

  return text;
}

// Update the island table using the current piece.
void _generate_islands_pos(board_t* board, state_cache_t* state, pos_t position, char color) {
  if (!is_valid_pos(position)) return;
  if (get_piece_color(get_piece(board, position)) != color) return;
  if (state->islands[position]) return;

  state->islands[position] = true;

  if (color)
    state->white_island_count++;
  else
    state->black_island_count++;

  for (int i=0; i<4; i++) {
    _generate_islands_pos(board, state, position + deltas[i], color);
  }
}

// Create the island table.
// This table can later be used to check if a move caused a piece to change an island.
void _generate_islands(board_t* board, state_cache_t* state) {

  // Count the number of island pieces.
  state->white_island_count = 0;
  state->black_island_count = 0;

  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      state->islands[to_position(row, col)] = false;
    }
  }

  // Generates state->islands on the center squares for both colors.
  _generate_islands_pos(board, state, 0x33, MOD_WHITE);
  _generate_islands_pos(board, state, 0x34, MOD_WHITE);
  _generate_islands_pos(board, state, 0x43, MOD_WHITE);
  _generate_islands_pos(board, state, 0x44, MOD_WHITE);
  _generate_islands_pos(board, state, 0x33, MOD_BLACK);
  _generate_islands_pos(board, state, 0x34, MOD_BLACK);
  _generate_islands_pos(board, state, 0x43, MOD_BLACK);
  _generate_islands_pos(board, state, 0x44, MOD_BLACK);

  // Quick check for if the number of island pieces are greater than the total number of pieces.
  assert(state->white_island_count <= state->white_count);
  assert(state->black_island_count <= state->black_count);

  // After updating the islands table, we must update the game status.
  _generate_board_status(board, state);
}

// Generate the square hashes table.
static inline void _generate_square_hash(state_cache_t *state) {
  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      for (int piece=0; piece<4; piece++) {
        state->square_hash[piece][to_position(row, col)] = rand();
      }
    }
  }
}

// Update the hash value after placing or removing a piece to a position.
static inline void _update_hash(state_cache_t* state, piece_t piece, pos_t pos) {
  state->hash ^= state->square_hash[piece - WHITE_PAWN][pos];
}

// Generate the hash value for a board.
static inline void _hash_board(board_t* board, state_cache_t* state) {
  state->hash = 0;
  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      pos_t pos = to_position(row, col);
      piece_t piece = get_piece(board, pos);

      if (piece != EMPTY)
        _update_hash(state, piece, pos);
    }
  }
}

// Generate a state cache from only the information given on the board.
void generate_state_cache(board_t* board, state_cache_t* state) {
  // Count the pieces on the board.
  state->white_count = 0;
  state->black_count = 0;

  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      char piece_color = get_piece_color(get_piece(board, to_position(row, col)));

      if (piece_color == MOD_WHITE)
        state->white_count++;
      else if (piece_color == MOD_BLACK)
        state->black_count++;
    }
  }

  // Update the islands.
  _generate_islands(board, state);

  // Update the game status.
  _generate_board_status(board, state);

  // Generate the hash value for the board.
  _generate_square_hash(state);
  _hash_board(board, state);
}

// Remove a piece on the board.
// Returns the removed piece.
// NOTE: Does not alter the state cache.
static inline piece_t _remove_piece(board_t* board, const state_cache_t* state, pos_t from, bool* update_islands_table) {
  // If the piece was moved from an island, table should be updated.
  if (!*update_islands_table && state->islands[from])
    *update_islands_table = true;

  // Take the piece from the origin position.
  piece_t piece = get_piece(board, from);

  // Clear the origin position.
  set_piece(board, from, EMPTY);

  // Return the removed piece.
  return piece;
}

// Place a piece to a position.
// NOTE: Does not alter the state cache.
static inline void _place_piece(board_t* board, const state_cache_t* state, pos_t to, piece_t piece, bool* update_islands_table) {
  // Set the destination position.
  set_piece(board, to, piece);

  if (!*update_islands_table &&
      (to == 0x33 ||
       to == 0x34 ||
       to == 0x43 ||
       to == 0x44))
    *update_islands_table = true;

  // Check for all of neighbors of the new position.
  if (!*update_islands_table) {
    for (int i=0; i<4; i++) {
      pos_t new_pos = to + deltas[i];

      if (!is_valid_pos(new_pos) ||
          get_piece_color(get_piece(board, new_pos)) != (board->turn ? MOD_WHITE : MOD_BLACK) ||
          !state->islands[new_pos])
        continue;

      *update_islands_table = true;
      break;
    }
  }
}

// Remove a piece from the board.
bool remove_piece(board_t* board, state_cache_t* state, pos_t pos) {
  bool update_islands_table = false;
  piece_t piece = _remove_piece(board, state, pos, &update_islands_table);

  _update_hash(state, piece, pos);
  char color = get_piece_color(piece);

  if (color == MOD_WHITE)
    state->white_count--;
  else if (color == MOD_BLACK)
    state->black_count--;
  else
    return false;

  if (update_islands_table)
    _generate_islands(board, state);

  _generate_board_status(board, state);

  return true;
}

// Place a piece on the board.
bool place_piece(board_t* board, state_cache_t* state, pos_t pos, piece_t piece) {
  _update_hash(state, piece, pos);
  char color = get_piece_color(piece);

  if (color == MOD_WHITE)
    state->white_count++;
  else if (color == MOD_BLACK)
    state->black_count++;
  else
    return false;

  if (get_piece(board, pos) != EMPTY)
    return false;

  bool update_islands_table = false;
  _place_piece(board, state, pos, piece, &update_islands_table);

  if (update_islands_table)
    _generate_islands(board, state);

  _generate_board_status(board, state);

  return true;
}

// Make a move on the board and update the state of the board.
// Both the board and move objects are assumed to be valid, so no checks are
// performed.
void do_move(board_t* board, state_cache_t* state, move_t move) {
  // There must not be any piece on the position where we are moving the piece.
  assert(get_piece(board, move.to) == EMPTY);

  bool update_islands_table = false;

  // Low level move the piece.
  // Piece color should be board->turn.
  piece_t piece = _remove_piece(board, state, move.from, &update_islands_table);
  assert(get_piece_color(piece) == (board->turn ? MOD_WHITE : MOD_BLACK));
  _place_piece(board, state, move.to, piece, &update_islands_table);

  _update_hash(state, piece, move.from);
  _update_hash(state, piece, move.to);

  // If the move is a capture move, remove the piece.
  // There must be a piece where we are going to capture of type capture_piece.
  if (is_capture(move)) {
    // Wtf is this!?
    #ifndef NDEBUG
    piece_t remove_piece =
    #endif

    _remove_piece(board, state, move.capture, &update_islands_table);
    assert(move.capture_piece == remove_piece);

    _update_hash(state, move.capture_piece, move.capture);
    char capture_color = get_piece_color(move.capture_piece);

    // Decrement the piece count.
    if (capture_color == MOD_WHITE)
      state->white_count--;
    else if (capture_color == MOD_BLACK)
      state->black_count--;
    else assert(false);
  }

  // Update the board turn and increment the move counter.
  next_turn(board);
  board->move_count++;

  // Update the necessary caches.
  if (update_islands_table) {
    _generate_islands(board, state);
  } else if (is_capture(move)) {
    _generate_board_status(board, state);
  }
}

// Undo a move on the board and update the state of the board.
void undo_move(board_t* board, state_cache_t* state, move_t move) {
  bool update_islands_table = false;

  // Low level move the piece.
  // Piece color should be !board->turn.
  piece_t piece = _remove_piece(board, state, move.to, &update_islands_table);
  assert(get_piece_color(piece) == (board->turn ? MOD_BLACK : MOD_WHITE));
  _place_piece(board, state, move.from, piece, &update_islands_table);

  _update_hash(state, piece, move.from);
  _update_hash(state, piece, move.to);

  // If the move is a capture move, add the piece.
  // There must be no piece where we are going to add the piece.
  if (is_capture(move)) {
    _place_piece(board, state, move.capture, move.capture_piece, &update_islands_table);

    _update_hash(state, move.capture_piece, move.capture);
    char capture_color = get_piece_color(move.capture_piece);

    // Increment the piece count.
    if (capture_color == MOD_WHITE) {
      state->white_count++;
    } else if (capture_color == MOD_BLACK) {
      state->black_count++;
    } else assert(false);
  }

  // Update the board turn and decrement the move counter.
  next_turn(board);
  board->move_count--;

  // Update the necessary caches.
  if (update_islands_table) {
    _generate_islands(board, state);
  } else if (is_capture(move)) {
    _generate_board_status(board, state);
  }
}
