#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "board.h"
#include "move.h"
#include "position.h"
#include "piece.h"

// Check if two moves are the same.
bool cmp_move(move_t move1, move_t move2) {
  if (move1.from != move2.from) return false;

  if (move1.to != move2.to) return false;

  if (is_valid_pos(move1.capture) ^ is_valid_pos(move2.capture)) return false;

  // move.capture_piece does not contain any information about the move, rather it has information about the board.
  // Because of that, we should expect them to be the same.
  // Otherwise means that the boards that these moves are generated from are different.
  assert(move1.capture_piece == move2.capture_piece);
  return true;
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

// Returns if the move is a capture move.
bool is_capture(move_t move) { return is_valid_pos(move.capture); }
