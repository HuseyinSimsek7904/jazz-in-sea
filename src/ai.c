#include "ai.h"
#include "position.h"

// Returns how many regular pawn moves it would take for the pawn to walk to the
// center of the board.
int pawn_dist_to_center(pos_t pos) {
  int col = to_col(pos);
  int row = to_row(pos);
  return (col <= 3 ? 3 - col : col - 4) + (row <= 3 ? 3 - row : row - 4);
}

// Returns how many regular knight moves it would take for the pawn to walk to
// the center of the board.
int knight_dist_to_center(pos_t pos) {
  int col = to_col(pos);
  int row = to_row(pos);
  return (col <= 3 ? (4 - col) / 2 : (col - 3) / 2) + (row <= 3 ? (4 - row) / 2 : (row - 3) / 2);
}
