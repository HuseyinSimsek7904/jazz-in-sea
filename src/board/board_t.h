#ifndef _BOARD_BOARD_T_H
#define _BOARD_BOARD_T_H

#include "board/piece_t.h"
#include "board/pos_t.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

// -- The board_t type --

typedef piece_t *board_t;


// -- Header functions --

// Get a piece at a position.
// Asserts if position is invalid.
static inline piece_t get_piece(board_t board, pos_t pos) {
#ifdef TEST_BOARD_INIT
  assert(board->initialized);
#endif

  assert(is_valid_pos(pos));
  return board[pos];
}

// Set a piece at a position.
// Asserts if position is invalid.
static inline void set_piece(board_t board, pos_t pos, piece_t piece) {
  assert(is_valid_pos(pos));
  board[pos] = piece;
}

#endif
