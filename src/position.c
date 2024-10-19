#include "position.h"

// Convert from (row, col) to position.
pos_t to_position(int row, int col) {
  // Encode position as:
  // 7  6  5  4  3  2  1  0
  // r3 r2 r1 r0 c3 c2 c1 c0
  return (row & 0x0f) << 4 | (col & 0x0f);
}

// Convert from position to row.
int to_row(pos_t pos) { return (pos & 0x70) >> 4; }

// Convert from position to column.
int to_col(pos_t pos) { return pos & 0x0f; }

// Check if a position is valid aka it is in the bounds.
bool is_valid_pos(pos_t pos) { return (pos & 0x88) == 0; }

bool is_center(pos_t pos) {
  int row = to_row(pos), col = to_col(pos);
  return (col == 3 || col == 4) && (row == 3 || row == 4);
}
