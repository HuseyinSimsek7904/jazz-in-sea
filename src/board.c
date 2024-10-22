#include <assert.h>
#include <stdbool.h>

#include "board.h"
#include "position.h"

void copy_board(board_t *src, board_t *dest) {
#ifndef NDEBUG
  dest->initialized = src->initialized;
#endif

  // Copy the turn value.
  dest->turn = src->turn;

  // Copy the board contents.
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      pos_t position = to_position(row, col);
      set_piece(dest, position, get_piece(src, position));
    }
  }
}

// Check if the boards are same.
bool compare(board_t *board1, board_t *board2) {
  // Uninitialized boards cannot be compared.
#ifndef NDEBUG
  assert(board1->initialized);
  assert(board2->initialized);
#endif

  if (board1->turn != board2->turn)
    return false;

  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      pos_t position = to_position(row, col);

      if (get_piece(board1, position) != get_piece(board2, position))
        return false;
    }
  }

  return true;
}
