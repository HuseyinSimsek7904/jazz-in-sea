#include <stdio.h>

#include "position.h"

// Convert from (row, col) to position.
int to_position(int row, int col) {
  // Encode position as:
  // 7  6  5  4  3  2  1  0
  // r3 r2 r1 r0 c3 c2 c1 c0
  return (row & 0x0f) << 4 | (col & 0x0f);
}

// Convert from position to row.
int to_row(int pos) {
  return (pos & 0xf0) >> 4;
}

// Convert from position to column.
int to_col(int pos) {
  return pos & 0xf0;
}

// Convert from regular row to perspective row or vice verca.
// Perspective row means the row that the player sees.
int perspective_row(int row, bool reverse) {
  // if reverse is true : perspective of the white player
  // if reverse is false: perspective of the black player
  return reverse ? row : 7 - row;
}

// Convert from regular column to perspective column or vice versa.
// Perspective col means the column that the player sees.
int perspective_col(int col, bool reverse) {
  // if reverse is true : perspective of the white player
  // if reverse is false: perspective of the black player
  return reverse ? 7 - col : col;
}

// Check if a position is valid aka it is in the bounds.
bool is_valid_pos(int pos) {
  return (pos & 0x88) == 0;
}
