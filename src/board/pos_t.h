#ifndef _BOARD_POSITION_T_H
#define _BOARD_POSITION_T_H

#include <stdbool.h>
#include <sys/types.h>

// The direction macros
#define UP    -0x10
#define DOWN  +0x10
#define LEFT  -0x01
#define RIGHT +0x01

static const int deltas[] = { RIGHT, DOWN, LEFT, UP };

#define INV_POSITION 0x88

// -- The pos_t type --
typedef u_int8_t pos_t;


// -- Header functions --

// Convert from (row, col) to position.
static inline pos_t to_position(int row, int col) {
  // Encode position as:
  // 7  6  5  4  3  2  1  0
  // r3 r2 r1 r0 c3 c2 c1 c0
  return (row & 0x0f) << 4 | (col & 0x0f);
}

// Convert from position to row.
static inline int to_row(pos_t pos) { return (pos & 0x70) >> 4; }

// Convert from position to column.
static inline int to_col(pos_t pos) { return pos & 0x0f; }

// Check if a position is valid aka it is in the bounds.
static inline bool is_valid_pos(pos_t pos) { return (pos & 0x88) == 0; }

static inline bool is_center(pos_t pos) {
  int row = to_row(pos), col = to_col(pos);
  return (col == 3 || col == 4) && (row == 3 || row == 4);
}

// Get the quadrant id of a position.
// This is useful for calculating the moves that get the piece closer to the
// center, which can be calculated by deltas[(i + quadrant) % 4]
// Quadrant IDs:
// 0 | 1
// --+--
// 3 | 2
static inline int get_quadrant(pos_t pos) {
  int quad = 0;

  if (to_col(pos) >= 4) {
    quad += 1;
  }

  if (to_row(pos) >= 4) {
    quad = 3 - quad;
  }

  return quad;
}

#endif
