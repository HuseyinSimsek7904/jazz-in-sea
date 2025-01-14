#ifndef _BOARD_H
#define _BOARD_H

#include "piece.h"
#include "position.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

// -- The board_t type --

typedef struct {
#if defined (TEST_BOARD_INIT) && ! defined (NDEBUG)
  bool initialized;
#endif

  // A board_array is a 16x16 array which contains information about each cell on a board.
  // The index of this array is called the POSITION.
  // A position can be a number between 0-255, however there are only 64 cells on a board.
  // Only the 0th-2nd and 4th-6th bits on a position is actually used, these is where the main board is located.
  // The remaining space is empty.
  // This extra space in positions are used for checking whether a position is valid.
  // So checking if (POS & 0x88) == 0 is enough to check if the position is out of the bounds of the board.
  // This will hopefully allow for fast checking of bounds.
  piece_t board_array[16 * 16];

  // false -> black, true -> white
  bool turn;
} board_t;


// -- Header functions --

// Get a piece at a position.
// Asserts if position is invalid.
static inline piece_t get_piece(board_t *board, pos_t pos) {
#ifdef TEST_BOARD_INIT
  assert(board->initialized);
#endif

  assert(is_valid_pos(pos));
  return board->board_array[pos];
}

// Set a piece at a position.
// Asserts if position is invalid.
static inline void set_piece(board_t *board, pos_t pos, piece_t piece) {
  assert(is_valid_pos(pos));
  board->board_array[pos] = piece;
}

// Change the turn of the board to the other player, and return the new turn.
static inline bool next_turn(board_t *board) { return board->turn = !board->turn; }

static inline bool whites_turn(board_t *board) { return  board->turn; }
static inline bool blacks_turn(board_t *board) { return !board->turn; }


// -- C functions --

bool compare_board(board_t*, board_t*);

#endif
