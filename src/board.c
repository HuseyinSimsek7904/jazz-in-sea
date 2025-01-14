#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>

#include "board.h"
#include "io.h"
#include "position.h"

// Check if the boards are same.
bool compare_board(board_t *board1, board_t *board2) {
  // Uninitialized boards cannot be compared.
#ifdef TEST_BOARD_INIT
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
