#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

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
  bool capture_available = false;
  int length = 0;

  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      unsigned int position = to_position(row, col);

      char piece = get_piece(board, position);

      // Check if the color of the piece is the color of the player.
      if (!is_piece_color(piece, board->turn)) continue;

      // Calculate the delta length of regular moves.
      int delta_mul = 1;
      if (is_piece_knight(piece)) {
        delta_mul = 2;
      }

      // Loop through all directions.
      for (size_t i=0; i<4; i++) {
        int delta = deltas[(i + get_quadrant(position)) % 4];

        // Calculate the first position and check if it is valid.
        int first_pos = position + delta * delta_mul;
        if (!is_valid_pos(first_pos)) continue;

        char first_piece = get_piece(board, first_pos);

        if (first_piece == ' ') {
          // The destination position is empty.
          // If there are any available captures, no need to try to find a regular move.
          if (capture_available) continue;

          // Set the move object.
          moves[length++] = (move_t) {
            .from = position,
            .to = first_pos,
            .capture = INV_POSITION,
          };

        } else if (is_piece_color(first_piece, !board->turn)) {
          // The destination position has a piece of opposite color and check if it is valid.
          int second_pos = first_pos + delta;
          if (!is_valid_pos(second_pos)) continue;

          // Check if the destination position is empty.
          if (get_piece(board, second_pos) != ' ') continue;

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

// Counts the cells in the island that contain this position.
unsigned int get_island_size(board_t* board, bool color, bool visited[256], unsigned int position) {
  // Check if the position is already visited.
  if (visited[position]) return 0;

  // Ignore the cells that do not contain pieces of the color we are interested in.
  if (!is_piece_color(get_piece(board, position), color)) return 0;

  visited[position] = true;

  // Add all of the neighbor cells' island size (we start with 1 to add the current cell to the count).
  unsigned int total = 1;
  for (int i=0; i<4; i++) {
    unsigned char new_position = position + deltas[i];

    if (is_valid_pos(new_position))
      total += get_island_size(board, color, visited, position + deltas[i]);
  }
  return total;
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
void _generate_islands_pos(board_t* board, state_cache_t* state, pos_t position, bool color) {
  if (!is_valid_pos(position)) return;

  if (!is_piece_color(get_piece(board, position), color)) return;

  if (state->islands[position]) return;

  state->islands[position] = true;

  if (color) {
    state->white_island_count++;
  } else {
    state->black_island_count++;
  }

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
  _generate_islands_pos(board, state, 0x33, false);
  _generate_islands_pos(board, state, 0x34, false);
  _generate_islands_pos(board, state, 0x43, false);
  _generate_islands_pos(board, state, 0x44, false);
  _generate_islands_pos(board, state, 0x33, true);
  _generate_islands_pos(board, state, 0x34, true);
  _generate_islands_pos(board, state, 0x43, true);
  _generate_islands_pos(board, state, 0x44, true);

  // Quick check for if the number of island pieces are greater than the total number of pieces.
  assert(state->white_island_count <= state->white_count);
  assert(state->black_island_count <= state->black_count);

  // After updating the islands table, we must update the game status.
  _generate_board_status(board, state);
}

// Generate a state cache from only the information given on the board.
void generate_state_cache(board_t* board, state_cache_t* state) {
  // Count the pieces on the board.
  state->white_count = 0;
  state->black_count = 0;

  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      char piece = get_piece(board, to_position(row, col));

      if (is_piece_white(piece)) {
        state->white_count++;
      } else if (is_piece_black(piece)) {
        state->black_count++;
      }
    }
  }

  // Update the islands.
  _generate_islands(board, state);

  // Update the game status.
  _generate_board_status(board, state);
}

// Remove a piece on the board.
// Returns the removed piece.
static inline char _remove_piece(board_t* board, state_cache_t* state, pos_t from, bool* update_islands_table) {
  // If the piece was moved from an island, table should be updated.
  if (!*update_islands_table && state->islands[from])
    *update_islands_table = true;

  // Take the piece from the origin position.
  // The piece must not be empty or of opposite color.
  char piece = get_piece(board, from);

  // Clear the origin position.
  set_piece(board, from, ' ');

  // Return the removed piece.
  return piece;
}

// Place a piece to a position.
static inline void _place_piece(board_t* board, state_cache_t* state, pos_t to, char piece, bool* update_islands_table) {
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
          !is_piece_color(get_piece(board, new_pos), board->turn) ||
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
  char piece = _remove_piece(board, state, pos, &update_islands_table);

  if (is_piece_white(piece)) {
    state->white_count--;
  } else if (is_piece_black(piece)) {
    state->black_count--;
  } else {
    return false;
  }

  if (update_islands_table)
    _generate_islands(board, state);

  _generate_board_status(board, state);

  return true;
}

// Place a piece on the board.
bool place_piece(board_t* board, state_cache_t* state, pos_t pos, char piece) {
  if (is_piece_white(piece)) {
    state->white_count++;
  } else if (is_piece_black(piece)) {
    state->black_count++;
  } else {
    return false;
  }

  if (get_piece(board, pos) != ' ') {
    return false;
  }

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
  assert(get_piece(board, move.to) == ' ');

  bool update_islands_table = false;

  // Low level move the piece.
  // Piece color should be board->turn.
  char piece = _remove_piece(board, state, move.from, &update_islands_table);
  assert(is_piece_color(piece, board->turn));
  _place_piece(board, state, move.to, piece, &update_islands_table);

  // If the move is a capture move, remove the piece.
  // There must be a piece where we are going to capture of type capture_piece.
  if (is_capture(move)) {
    assert(move.capture_piece == _remove_piece(board, state, move.capture, &update_islands_table));

    // Decrement the piece count.
    if (is_piece_white(move.capture_piece)) {
      state->white_count--;
    } else {
      assert(is_piece_black(move.capture_piece));
      state->black_count--;
    }
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
  char piece = _remove_piece(board, state, move.to, &update_islands_table);
  assert(is_piece_color(piece, !board->turn));
  _place_piece(board, state, move.from, piece, &update_islands_table);

  // If the move is a capture move, add the piece.
  // There must be no piece where we are going to add the piece.
  if (is_capture(move)) {
    _place_piece(board, state, move.capture, move.capture_piece, &update_islands_table);

    // Increment the piece count.
    if (is_piece_white(move.capture_piece)) {
      state->white_count++;
    } else {
      assert(is_piece_black(move.capture_piece));
      state->black_count++;
    }
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

// Generate the square hashes table.
void generate_square_hash(state_cache_t *state) {
  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      for (int piece=0; piece<4; piece++) {
        state->square_hash[piece][to_position(row, col)] = rand();
      }
    }
  }
}

static inline int piece_id(char piece) {
  switch (piece) {
  case 'P':
    return 0;
  case 'N':
    return 1;
  case 'p':
    return 2;
  case 'n':
    return 3;
  default:
    assert(false);
    return 0;
  }
}

unsigned short hash_board(board_t* board, state_cache_t* state) {
  unsigned short hash = 0;
  for (int row=0; row<8; row++) {
    for (int col=0; col<8; col++) {
      pos_t pos = to_position(row, col);
      char piece = get_piece(board, pos);

      if (piece != ' ')
        hash ^= state->square_hash[piece_id(piece)][pos];
    }
  }
  return hash;
}
