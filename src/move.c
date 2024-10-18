#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "board.h"
#include "move.h"
#include "position.h"
#include "piece.h"

// Print a move.
// The notation for regular moves is: <from_position> > <to_row_or_col_name>
// The notation for capture moves is: <from_position> x <to_row_or_col_name>
// If the piece moved horizontally, <to_row_or_col_name> is the to column name.
// If the piece moved vertically, <to_row_or_col_name> is the to row name.
void print_move(move_t move) {
  // Check if the move is valid.
  if (!is_valid_move(move)) {
    printf("<invalid move>");
    return;
  }

  print_position(move.from);
  printf("%c", is_valid_pos(move.capture) ? 'x' : '>');

  if (to_col(move.from) == to_col(move.to)) {
    // Column numbers are equal, so the piece moved vertically.
    printf("%c", row_name(to_row(move.to)));
  } else {
    // This should be true according to the normal Cez rules.
    // Because pieces move only horizontally or vertically.
    assert(to_row(move.from) == to_row(move.to));

    printf("%c", col_name(to_col(move.to)));
  }
}

// Check if a move is valid.
// This can be checked by checking if the from position is valid.
bool is_valid_move(move_t move) {
  return is_valid_pos(move.from);
}

// Make a move on the board.
// Both the board and move objects are assumed to be valid, so no checks are
// performed.
void do_move(board_t* board, move_t move) {
  // Take the piece from the origin position.
  // The piece must not be empty or of opposite color.
  char piece = get_piece(board, move.from);
  assert(is_piece_color(piece, board->turn));

  // Clear the origin position.
  set_piece(board, move.from, ' ');

  // Set the destination position.
  // There must not be any piece on the position where we are moving the piece.
  assert(get_piece(board, move.to) == ' ');
  set_piece(board, move.to, piece);

  // If the move is a capture move, remove the piece.
  // There must be a piece where we are going to capture of type capture_piece.
  if (is_capture(move)) {
    assert(get_piece(board, move.capture) == move.capture_piece);
    set_piece(board, move.capture, ' ');
  }

  // Update the board turn.
  next_turn(board);
}

// Undo a move on the board.
void undo_move(board_t* board, move_t move) {
  // Take the piece from the destination position.
  // The piece must not be empty or of opposite color.
  char piece = get_piece(board, move.to);
  assert(is_piece_color(piece, !board->turn));

  // Clear the destination position.
  set_piece(board, move.to, ' ');

  // Set the origin position.
  // There must not be any piece on the position where we are moving the piece.
  assert(get_piece(board, move.from) == ' ');
  set_piece(board, move.from, piece);

  // If the move is a capture move, add the piece.
  // There must be no piece where we are going to add the piece.
  if (is_capture(move)) {
    assert(get_piece(board, move.capture) == ' ');
    set_piece(board, move.capture, move.capture_piece);
  }

  // Update the board turn.
  next_turn(board);
}

// Returns if the move is a capture move.
bool is_capture(move_t move) { return is_valid_pos(move.capture); }
