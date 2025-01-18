#ifndef _MOVE_MOVE_T_H
#define _MOVE_MOVE_T_H

#include "board/board_t.h"
#include "board/piece_t.h"
#include "board/pos_t.h"

// Direction units.
// These macros are defined using the perspective of the white player.
typedef struct {
  // The position from where the piece moved.
  pos_t from;

  // The position where the piece moved to.
  pos_t to;

  // The position of the piece that the moving piece captured.
  // INV_POSITION if no capture.
  pos_t capture;

  // The type of the piece that the moving piece captured.
  // Does not matter if no capture.
  piece_t capture_piece;
} move_t;

#define INV_MOVE (move_t) { .from=INV_POSITION }

// Check if two moves are the same.
static inline bool compare_move(move_t move1, move_t move2) {
  if (move1.from != move2.from) return false;

  if (move1.to != move2.to) return false;

  if (is_valid_pos(move1.capture) ^ is_valid_pos(move2.capture)) return false;

  // move.capture_piece does not contain any information about the move, rather it has information about the board.
  // Because of that, we should expect them to be the same.
  // Otherwise means that the boards that these moves are generated from are different.

  assert(!is_valid_pos(move1.capture) || move1.capture_piece == move2.capture_piece);
  return true;
}

// Check if a move is valid.
// This can be checked by checking if the from position is valid.
static inline bool is_valid_move(move_t move) {
  return is_valid_pos(move.from);
}

// Returns if the move is a capture move.
static inline bool is_capture(move_t move) { return is_valid_pos(move.capture); }

#endif
