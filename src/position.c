#include <stdio.h>

#include "position.h"

// Convert from (row, col) to position.
inline pos_t to_position(int row, int col) {
  // Encode position as:
  // 7  6  5  4  3  2  1  0
  // r3 r2 r1 r0 c3 c2 c1 c0
  return (row & 0x0f) << 4 | (col & 0x0f);
}

// Convert from position to row.
inline int to_row(pos_t pos) { return (pos & 0x70) >> 4; }

// Convert from position to column.
inline int to_col(pos_t pos) { return pos & 0x0f; }

// Convert from regular row to perspective row or vice verca.
// Perspective row means the row that the player sees.
inline int perspective_row(int row, bool reverse) {
  // if reverse is true : perspective of the white player
  // if reverse is false: perspective of the black player
  return reverse ? row : 7 - row;
}

// Convert from regular column to perspective column or vice versa.
// Perspective col means the column that the player sees.
inline int perspective_col(int col, bool reverse) {
  // if reverse is true : perspective of the white player
  // if reverse is false: perspective of the black player
  return reverse ? 7 - col : col;
}

// Check if a position is valid aka it is in the bounds.
inline bool is_valid_pos(pos_t pos) { return (pos & 0x88) == 0; }

// Get the column name from a column number.
// aka 0->a, 1->b ...  7->h
inline char col_name(int col) { return 'a' + col; }

// Get the row name from a row number.
// aka 0->1, 1->2, ... 7->8
inline char row_name(int row) { return '1' + row; }

// Print a position.
// The notation is: <column-name><row-number>
inline void print_position(pos_t pos) {
  printf("%c%c", col_name(to_col(pos)), row_name(to_row(pos)));
}

inline bool is_center(pos_t pos) {
  int row = to_row(pos), col = to_col(pos);
  return (col == 3 || col == 4) && (row == 3 || row == 4);
}
