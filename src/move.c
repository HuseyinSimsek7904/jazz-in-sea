#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "board.h"
#include "move.h"
#include "position.h"
#include "piece.h"

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

  // Update the board turn and increment the move counter.
  next_turn(board);
  board->move_count++;
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

  // Update the board turn and decrement the move counter.
  next_turn(board);
  board->move_count--;
}
