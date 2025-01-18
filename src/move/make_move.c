#include "state/hash_operations.h"
#include "state/state_generation.h"
#include "state/status.h"
#include "move/make_move.h"

// Remove a piece on the board.
// Returns the removed piece.
// NOTE: Does not alter the state cache.
static inline piece_t _remove_piece(board_state_t* state, pos_t from, bool* update_islands_table) {
  // If the piece was moved from an island, table should be updated.
  if (!*update_islands_table && state->islands[from])
    *update_islands_table = true;

  // Take the piece from the origin position.
  piece_t piece = get_piece(state->board, from);

  // Clear the origin position.
  set_piece(state->board, from, EMPTY);

  // Return the removed piece.
  return piece;
}

// Place a piece to a position.
// NOTE: Does not alter the state cache.
static inline void _place_piece(board_state_t* state, pos_t to, piece_t piece, bool* update_islands_table) {
  // Set the destination position.
  set_piece(state->board, to, piece);

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

      if (is_valid_pos(new_pos) &&
          get_piece_color(get_piece(state->board, new_pos)) == get_piece_color(piece) &&
          state->islands[new_pos]) {

        *update_islands_table = true;
        break;
      }
    }
  }
}

// Remove a piece from the board.
// Clears the history.
bool remove_piece(board_state_t* state, history_t* history, pos_t pos) {
  bool update_islands_table = false;
  piece_t piece = _remove_piece(state, pos, &update_islands_table);

  update_hash_for_piece(state, piece, pos);
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
bool place_piece(board_state_t* state, history_t* history, pos_t pos, piece_t piece) {
  update_hash_for_piece(state, piece, pos);
  char color = get_piece_color(piece);

  if (color == MOD_WHITE)
    state->white_count++;
  else if (color == MOD_BLACK)
    state->black_count++;
  else
    return false;

  if (get_piece(state->board, pos) != EMPTY)
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
// Both the state->board and move objects are assumed to be valid, so no checks are
// performed.
void do_move(board_state_t* state, history_t* history, move_t move) {
  // Add the move and the old state->board to the history.
  history->history[history->size++] = (history_item_t) {
    .move = move,
    .hash = state->hash,
  };

  // There must not be any piece on the position where we are moving the piece.
  assert(get_piece(state->board, move.to) == EMPTY);

  bool update_islands_table = false;

  // Low level move the piece.
  // Piece color should be state->board->turn.
  piece_t piece = _remove_piece(state, move.from, &update_islands_table);
  assert(get_piece_color(piece) == (state->turn ? MOD_WHITE : MOD_BLACK));
  _place_piece(state, move.to, piece, &update_islands_table);

  update_hash_for_piece(state, piece, move.from);
  update_hash_for_piece(state, piece, move.to);
  state->hash ^= state->turn_hash;

  // If the move is a capture move, remove the piece.
  // There must be a piece where we are going to capture of type capture_piece.
  if (is_capture(move)) {
    // Wtf is this!?
    #ifndef NDEBUG
    piece_t remove_piece =
    #endif

    _remove_piece(state, move.capture, &update_islands_table);
    assert(move.capture_piece == remove_piece);

    update_hash_for_piece(state, move.capture_piece, move.capture);
    char capture_color = get_piece_color(move.capture_piece);

    // Decrement the piece count.
    if (capture_color == MOD_WHITE)
      state->white_count--;
    else if (capture_color == MOD_BLACK)
      state->black_count--;
    else assert(false);
  }

  // Update the turn.
  state->turn = !state->turn;

  // Update the necessary caches.
  if (update_islands_table) {
    generate_islands(state);
  }

  generate_board_status(state, history);
}

// Undo a move on the state->board and update the state of the state->board.
void undo_last_move(board_state_t* state, history_t* history) {
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

  update_hash_for_piece(state, piece, move.from);
  update_hash_for_piece(state, piece, move.to);
  state->hash ^= state->turn_hash;

  // If the move is a capture move, add the piece.
  // There must be no piece where we are going to add the piece.
  if (is_capture(move)) {
    _place_piece(state, move.capture, move.capture_piece, &update_islands_table);

    update_hash_for_piece(state, move.capture_piece, move.capture);
    char capture_color = get_piece_color(move.capture_piece);

    // Increment the piece count.
    if (capture_color == MOD_WHITE) {
      state->white_count++;
    } else if (capture_color == MOD_BLACK) {
      state->black_count++;
    } else assert(false);
  }

  // Update the turn.
  state->turn = !state->turn;

  // Update the necessary caches.
  if (update_islands_table) {
    generate_islands(state);
  }

  generate_board_status(state, history);
}
